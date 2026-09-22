#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace turtle {

template <typename Key, typename Value> class CuckooHashTable {
public:
  using Entry = std::pair<Key, Value>;

  explicit CuckooHashTable(size_t capacity)
      : capacity_(capacity), max_iterations_(capacity) {
    this->table1_ = new std::optional<Entry>[capacity];
    this->table2_ = new std::optional<Entry>[capacity];
    this->rehash_seeds();
  }

  ~CuckooHashTable() {
    delete[] table1_;
    delete[] table2_;
  }

  /**
   * O(1) worst case - checks exactly 2 positions.
   * Returns nullptr if the key is not present.
   */
  Value *lookup(const Key &key) {
    size_t pos1 = this->h1(key);
    if (this->table1_[pos1].has_value() && this->table1_[pos1]->first == key) {
      return &this->table1_[pos1]->second;
    }

    size_t pos2 = this->h2(key);
    if (this->table2_[pos2].has_value() && this->table2_[pos2]->first == key) {
      return &this->table2_[pos2]->second;
    }

    return nullptr;
  }

  bool insert(const Key &key, const Value &value) {
    // Update existing key (O(1))
    Value *existing = this->lookup(key);
    if (existing != nullptr) {
      *existing = value;
      return true;
    }

    // New key: snapshot before the eviction chain modifies the tables
    Entry entry{key, value};
    std::vector<Entry> entries = this->all_entries();

    if (!this->try_insert(entry)) {
      entries.push_back(entry);
      this->rehash(entries);
    }

    return true;
  }

  bool remove(const Key &key) {
    size_t pos1 = this->h1(key);
    if (this->table1_[pos1].has_value() && this->table1_[pos1]->first == key) {
      this->table1_[pos1].reset();
      this->size_--;
      return true;
    }

    size_t pos2 = this->h2(key);
    if (this->table2_[pos2].has_value() && this->table2_[pos2]->first == key) {
      this->table2_[pos2].reset();
      this->size_--;
      return true;
    }

    return false;
  }

private:
  /** @brief number of slots per table (total slots = 2 * capacity) */
  size_t capacity_{0};

  /** @brief eviction chain length before declaring a cycle */
  size_t max_iterations_;

  /** @brief raw pointer to table1_ heap memory */
  std::optional<Entry> *table1_;

  /** @brief raw pointer to table2_ heap memory */
  std::optional<Entry> *table2_;

  size_t size_{0};

  uint64_t seed1_;
  uint64_t seed2_;

  // Re-randomizes seeds on rehash to break cycles or adversarial patterns
  void rehash_seeds() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, static_cast<uint64_t>(1e6));

    this->seed1_ = dis(gen);
    this->seed2_ = dis(gen);
  }

  // Helper to mix a has value with a runtime seed
  size_t mix_hash(size_t base_hash, uint64_t seed) const {
    return base_hash ^
           (seed + 0x9e3779b97f4a7c15ULL + (base_hash << 6) + (base_hash >> 2));
  }

  // First hash function using seed1_
  size_t h1(const Key &key) const {
    std::hash<Key> hasher;
    return this->mix_hash(hasher(key), this->seed1_) % this->capacity_;
  }

  // second hash function using seed1_
  size_t h2(const Key &key) const {
    std::hash<Key> hasher;
    return this->mix_hash(hasher(key), this->seed2_) % this->capacity_;
  }

  std::vector<Entry> all_entries() const {
    std::vector<Entry> result;

    // Pre-allocate memory to avoid reallocations
    // (maximum possible entries = 2 * capacity)
    result.reserve(this->capacity_ * 2);

    // Scan table 1
    for (size_t i = 0; i < this->capacity_; ++i) {
      if (this->table1_[i].has_value())
        result.push_back(*this->table1_[i]);
    }

    // Scan table 2
    for (size_t i = 0; i < this->capacity_; ++i) {
      if (this->table2_[i].has_value())
        result.push_back(*this->table2_[i]);
    }

    return result;
  }

  /**
   * Cuckoo eviction loop — O(max_iterations) = O(capacity).
   */
  bool try_insert(Entry entry) {
    Entry cur_entry = entry;

    for (size_t i = 0; i < this->max_iterations_; ++i) {
      // Try table1 first
      size_t pos1 = this->h1(cur_entry.first);
      if (!this->table1_[pos1].has_value()) {
        this->table1_[pos1] = cur_entry;
        this->size_++;
        return true;
      }

      // Slot occupied — evict current resident and continue with it
      Entry evicted = *this->table1_[pos1];
      this->table1_[pos1] = cur_entry;
      cur_entry = evicted;

      // Try table2 with the evicted item
      size_t pos2 = this->h2(cur_entry.first);
      if (!this->table2_[pos2].has_value()) {
        this->table2_[pos2] = cur_entry;
        this->size_++;
        return true;
      }

      // Slot occupied - evict and loop
      evicted = *this->table2_[pos2];
      this->table2_[pos2] = cur_entry;
      cur_entry = evicted;
    }

    return false;
  }

  /**
   * O(n) — rebuilds both tables from scratch with new hash seeds.
   * Strategy:
   *  1. If load > 40%, double capacity before reinserting (reduces future
   *     cycle probability). Otherwise just reseed.
   *  2. Retry up to 5 times. If reseeding alone doesn't help, grow on
   *     every subsequent attempt.
   *  3. Raise RuntimeError if all attempts fail (practically impossible
   *     at reasonable load factors).
   *
   * Why 40% threshold? At load > 50% total (each table > 50% full), the
   * probability of an insertion cycle rises sharply. 40% gives headroom.
   */
  void rehash(std::vector<Entry> &entries) {
    try {
      bool grow =
          static_cast<double>(entries.size()) / (2 * this->capacity_) > 0.4;

      for (size_t i = 0; i <= 5; ++i) {
        if (grow) {
          this->capacity_ *= 2;
          this->max_iterations_ = this->capacity_;
        }

        delete[] this->table1_;
        delete[] this->table2_;

        // New seeds break any hash-pattern that caused the cycle
        this->rehash_seeds();
        this->table1_ = new std::optional<Entry>[this->capacity_];
        this->table2_ = new std::optional<Entry>[this->capacity_];

        this->size_ = 0;

        bool success = std::all_of(
            entries.begin(), entries.end(),
            [this](auto const &entry) { return this->try_insert(entry); });

        if (success) {
          return;
        }

        // reseeding alone failed; grow before next attempt
        grow = true;
      }
    } catch (const std::runtime_error &) {
      throw std::runtime_error(
          "Cuckoo hashing failed to rehash after multiple attempts");
    }
  }
};

} // namespace turtle
