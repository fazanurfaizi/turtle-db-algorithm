#include "turtledb/memory/lru_replacer.h"
#include "gtest/gtest.h"
#include <stdexcept>

const int NUM_OF_TEST_RECORDS = 100;
const int CACHE_CAPACITY = 50;

TEST(LRUReplacerTest, SimplePut) {
  turtle::LRUCache<int32_t> cache(1);
  cache.put(2, 100);
  ASSERT_TRUE(cache.exists(2));
  ASSERT_EQ(cache.get(2), 100);
  ASSERT_EQ(cache.size(), 1);
}

TEST(LRUReplacerTest, MissingValue) {
  turtle::LRUCache<int> cache(1);
  EXPECT_THROW(cache.get(22), std::range_error);
}

TEST(LRUReplacerTest, KeepsValuesByCapacity) {
  turtle::LRUCache<int16_t> cache(CACHE_CAPACITY);

  for (size_t i = 0; i < NUM_OF_TEST_RECORDS; ++i) {
    cache.put(i, i);
  }

  for (size_t i = 0; i < NUM_OF_TEST_RECORDS - CACHE_CAPACITY; ++i) {
    EXPECT_FALSE(cache.exists(i));
  }

  for (int i = NUM_OF_TEST_RECORDS - CACHE_CAPACITY; i < NUM_OF_TEST_RECORDS;
       ++i) {
    EXPECT_TRUE(cache.exists(i));
    EXPECT_EQ(cache.get(i), i);
  }

  EXPECT_EQ(cache.size(), CACHE_CAPACITY);
}
