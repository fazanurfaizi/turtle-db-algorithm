#pragma once

#include <cstdint>
#include <stdexcept>
#include <unordered_map>

namespace turtle {
template <typename T> struct Node {
  int32_t key_;
  T value_;
  Node *prev_;
  Node *next_;

  Node(int32_t key, T value)
      : key_(key), value_(value), prev_(nullptr), next_(nullptr) {}
};

template <typename T> class LRUCache {
public:
  LRUCache(int32_t capacity) : capacity_(capacity) {
    this->head_ = new Node<T>(0, T{});
    this->tail_ = new Node<T>(0, T{});
    this->head_->next_ = this->tail_;
    this->tail_->prev_ = this->head_;
  }

  void put(int32_t key, T value) {
    auto it = this->cached_items_.find(key);
    if (it != this->cached_items_.end()) {
      Node<T> *node = it->second;
      remove(node);
      insert(node);
    } else {
      if (this->size() == this->capacity_) {
        Node<T> *lru = this->tail_->prev_;
        this->cached_items_.erase(lru->key_);
        this->remove(lru);
        delete lru;
      }
      Node<T> *node = new Node<T>(key, value);
      this->cached_items_[key] = node;
      this->insert(node);
    }
  }

  T &get(int32_t key) {
    auto it = this->cached_items_.find(key);
    if (it == this->cached_items_.end()) {
      throw std::range_error("There is no such key in cache");
    } else {
      Node<T> *node = it->second;
      remove(node);
      insert(node);
      return node->value_;
    }
  }

  bool exists(int32_t key) const {
    return this->cached_items_.find(key) != this->cached_items_.end();
  }

  int32_t size() { return this->cached_items_.size(); }

private:
  int32_t capacity_;
  std::unordered_map<int32_t, Node<T> *> cached_items_;
  Node<T> *head_;
  Node<T> *tail_;

  void remove(Node<T> *node) {
    node->prev_->next_ = node->next_;
    node->next_->prev_ = node->prev_;
  }

  void insert(Node<T> *node) {
    node->next_ = this->head_->next_;
    node->next_->prev_ = node;
    this->head_->next_ = node;
    node->prev_ = head_;
  }
};

} // namespace turtle
