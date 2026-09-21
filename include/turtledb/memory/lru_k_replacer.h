#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <list>
#include <stdexcept>
#include <unordered_map>

namespace turtle {
template <typename T> struct Node {
  int32_t key_;
  T value_;
  std::list<size_t> histories_;

  Node(int32_t key, T value) : key_(key), value_(value) {}
};

template <typename T> class LRUKReplacer {
public:
  LRUKReplacer(size_t capacity, size_t k) : capacity_(capacity), k_(k) {}

  void put(int32_t key, T value) {
    auto it = this->caches_.find(key);
    if (it != this->caches_.end()) {
      // cache already saved
      it->second->value_ = value;
      this->record_access(it->second);
    } else {
      // create new cache
      if (this->size() == this->capacity_) {
        // evict lowest used cache
        this->evict();
      }

      Node<T> *node = new Node<T>(key, value);
      this->record_access(node);
      this->caches_[key] = node;
    }
  }

  T &get(int32_t key) {
    auto it = this->caches_.find(key);
    if (it == this->caches_.end()) {
      throw std::range_error("There is no such key in cache");
    } else {
      Node<T> *node = it->second;
      this->record_access(node);
      return node->value_;
    }
  }

  bool exists(int32_t key) const {
    return this->caches_.find(key) != this->caches_.end();
  }

  size_t size() { return this->caches_.size(); }

private:
  size_t capacity_;
  size_t k_;
  size_t current_timestamp_{0};
  std::unordered_map<int32_t, Node<T> *> caches_;

  void record_access(Node<T> *node) {
    node->histories_.push_back(this->current_timestamp_);
    if (node->histories_.size() >= this->k_) {
      node->histories_.pop_front();
    }
    this->current_timestamp_++;
  }

  void evict() {
    if (this->caches_.empty()) {
      return;
    }

    bool found = false;

    size_t victim = 0;
    size_t max_distance = -1;
    size_t earliest_ts = std::numeric_limits<size_t>::max();

    for (auto &[key, node] : this->caches_) {
      size_t distance;
      if (node->histories_.size() < this->k_) {
        distance = std::numeric_limits<size_t>::max();
      } else {
        distance = this->current_timestamp_ - node->histories_.front();
      }

      size_t first_ts = node->histories_.front();

      bool better = !found || distance > max_distance ||
                    (distance == max_distance &&
                     distance == std::numeric_limits<size_t>::max() &&
                     first_ts < earliest_ts);

      if (better) {
        found = true;
        max_distance = distance;
        earliest_ts = first_ts;
        victim = key;
      }
    }

    this->caches_.erase(victim);
  }
};

} // namespace turtle
