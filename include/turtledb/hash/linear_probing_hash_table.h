#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>

namespace turtle {

template <typename Key, typename Value> struct HashNode {
  enum class State { EMPTY, OCCUPIED, DELETED };

  Key key_{};
  Value value_{};
  State state_{State::EMPTY};

  HashNode() = default;
  HashNode(Key key, Value value)
      : key_(std::move(key)), value_(std::move(value)),
        state_(State::OCCUPIED) {}
};

template <typename Key, typename Value> class LinearProbingHash {
public:
  LinearProbingHash(size_t capacity) : capacity_(capacity) {
    this->arr_ = new HashNode<Key, Value> *[capacity];
    std::fill_n(this->arr_, capacity, nullptr);
  }

  ~LinearProbingHash() {
    this->clear();
    delete[] this->arr_;
  }

  // Prevent copying for raw pointer management
  LinearProbingHash(const LinearProbingHash &) = delete;
  LinearProbingHash &operator=(const LinearProbingHash &) = delete;

  void insert(Key key, Value value) {
    // rehash if load factor exceeds 0.7
    if (static_cast<double>(this->size_) / this->capacity_ >= 0.7) {
      this->rehash();
    }

    size_t hash_index = this->hash_code(key);
    size_t first_deleted_index = this->capacity_;
    size_t counter = 0;

    // Linear probing to find an empty or deleted slot
    while (this->arr_[hash_index] != nullptr) {
      if (counter++ >= this->capacity_) {
        return;
      }

      // Existing key match: update value in place
      if (this->arr_[hash_index]->state_ ==
              HashNode<Key, Value>::State::OCCUPIED &&
          this->arr_[hash_index]->key_ == key) {
        this->arr_[hash_index]->value_ = std::move(value);
        return;
      }

      // Remember the first deleted slot to recycle it
      if (this->arr_[hash_index]->state_ ==
              HashNode<Key, Value>::State::DELETED &&
          first_deleted_index == this->capacity_) {
        first_deleted_index = hash_index;
      }

      hash_index = (hash_index + 1) % this->capacity_;
    }

    // reuse deleted slot if available, otherwise use empty slot
    size_t target_index = (first_deleted_index != this->capacity_)
                              ? first_deleted_index
                              : hash_index;

    if (this->arr_[target_index] == nullptr) {
      this->arr_[target_index] =
          new HashNode<Key, Value>(std::move(key), std::move(value));
    } else {
      this->arr_[target_index]->key_ = std::move(key);
      this->arr_[target_index]->value_ = std::move(value);
      this->arr_[target_index]->state_ = HashNode<Key, Value>::State::OCCUPIED;
    }

    this->size_++;
  }

  std::optional<Value> get(Key key) {
    size_t hash_index = this->hash_code(key);
    size_t counter = 0;

    // Linear probing to find the key
    while (this->arr_[hash_index] != nullptr) {
      if (counter++ >= this->capacity_) {
        return std::nullopt;
      }

      if (this->arr_[hash_index]->state_ ==
              HashNode<Key, Value>::State::OCCUPIED &&
          this->arr_[hash_index]->key_ == key) {
        return this->arr_[hash_index]->value_;
      }

      hash_index = (hash_index + 1) % this->capacity_;
    }

    return std::nullopt;
  }

  bool remove(Key key) {
    size_t hash_index = this->hash_code(key);
    size_t counter = 0;

    while (this->arr_[hash_index] != nullptr) {
      if (counter++ >= this->capacity_) {
        return false;
      }

      if (this->arr_[hash_index]->state_ ==
              HashNode<Key, Value>::State::OCCUPIED &&
          this->arr_[hash_index]->key_ == key) {
        this->arr_[hash_index]->state_ = HashNode<Key, Value>::State::DELETED;
        this->size_--;
        return true;
      }

      hash_index = (hash_index + 1) % this->capacity_;
    }

    return false;
  }

  [[nodiscard]] size_t size() const { return this->size_; }
  [[nodiscard]] size_t capacity() const { return this->capacity_; }

private:
  /** @brief Maximum size of the hash table */
  size_t capacity_;

  /** @brief Current number of elements in the map */
  size_t size_{0};

  /** @brief Array of pointers to HashNode */
  HashNode<Key, Value> **arr_;

  size_t hash_code(Key key) {
    std::hash<Key> hasher;
    return hasher(key) % this->capacity_;
  }

  void clear() {
    for (size_t i = 0; i < this->capacity_; ++i) {
      delete this->arr_[i];
      this->arr_[i] = nullptr;
    }
    this->size_ = 0;
  }

  void rehash() {
    HashNode<Key, Value> **old_arr = this->arr_;
    size_t old_cap = this->capacity_;

    this->capacity_ *= 2;
    this->size_ = 0;
    this->arr_ = new HashNode<Key, Value> *[this->capacity_];
    std::fill_n(this->arr_, this->capacity_, nullptr);

    // Re-insert all old elements into the new array
    for (size_t i = 0; i < old_cap; i++) {
      if (old_arr[i] != nullptr) {
        if (old_arr[i]->state_ == HashNode<Key, Value>::State::OCCUPIED) {
          this->insert(std::move(old_arr[i]->key_),
                       std::move(old_arr[i]->value_));
        }
        delete old_arr[i];
      }
    }

    delete[] old_arr;
  }
};

} // namespace turtle
