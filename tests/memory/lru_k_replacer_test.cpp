#include "turtledb/memory/lru_k_replacer.h"
#include "gtest/gtest.h"
#include <stdexcept>

const int NUM_OF_TEST_RECORDS = 100;
const int CACHE_CAPACITY = 50;
const int K = 2;

TEST(LRUKReplacerTest, SimplePut) {
  turtle::LRUKReplacer<int32_t> cache(1, K);
  cache.put(2, 100);
  ASSERT_TRUE(cache.exists(2));
  ASSERT_EQ(cache.get(2), 100);
  ASSERT_EQ(cache.size(), 1);
}

TEST(LRUKReplacerTest, MissingValue) {
  turtle::LRUKReplacer<int> cache(1, K);
  EXPECT_THROW(cache.get(22), std::range_error);
}

TEST(LRUKReplacerTest, UpdateExistingKeyKeepsSize) {
  turtle::LRUKReplacer<int32_t> cache(2, K);
  cache.put(1, 10);
  cache.put(1, 11);
  EXPECT_EQ(cache.size(), 1);
  EXPECT_EQ(cache.get(1), 11);
}

TEST(LRUKReplacerTest, KeepsValuesByCapacity) {
  turtle::LRUKReplacer<int16_t> cache(CACHE_CAPACITY, K);

  for (size_t i = 0; i < NUM_OF_TEST_RECORDS; ++i) {
    cache.put(i, i);
  }

  EXPECT_EQ(cache.size(), CACHE_CAPACITY);
}

TEST(LRUKReplacerTest, EvictsUnderKPageBeforeEstablishedPage) {
  turtle::LRUKReplacer<int32_t> cache(3, K);

  cache.put(1, 10);
  cache.put(1, 11);
  cache.put(2, 20);
  cache.put(3, 30);

  EXPECT_EQ(cache.get(1), 11);
  EXPECT_TRUE(cache.exists(1));
  EXPECT_TRUE(cache.exists(2));
  EXPECT_TRUE(cache.exists(3));
}

TEST(LRUKReplacerTest, EvictsLargestKDistanceWhenAllEstablished) {
  turtle::LRUKReplacer<int32_t> cache(2, K);

  cache.put(1, 10);
  cache.put(2, 20);
  cache.get(1);
  cache.get(2);
  cache.get(1);
  cache.get(2);
  cache.put(3, 30);

  EXPECT_FALSE(cache.exists(1));
  EXPECT_TRUE(cache.exists(2));
  EXPECT_TRUE(cache.exists(3));
}
