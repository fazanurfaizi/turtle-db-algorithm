#include "turtledb/hash/cuckoo_hash_table.h"
#include "gtest/gtest.h"
#include <string>
#include <utility>
#include <vector>

namespace turtle {

namespace test {

class CuckooHashTableTest : public ::testing::Test {};

TEST_F(CuckooHashTableTest, LookupNonExistingKey) {
  CuckooHashTable<int, std::string> ht(8);

  EXPECT_EQ(ht.lookup(42), nullptr);
  EXPECT_EQ(ht.lookup(-1), nullptr);
}

TEST_F(CuckooHashTableTest, InsertAndLookupSingleItem) {
  CuckooHashTable<int, std::string> ht(8);

  EXPECT_TRUE(ht.insert(10, "TurtleDB"));

  auto *val = ht.lookup(10);
  ASSERT_NE(val, nullptr);
  EXPECT_EQ(*val, "TurtleDB");
}

TEST_F(CuckooHashTableTest, InsertAndLookupMultipleItems) {
  CuckooHashTable<int, std::string> ht(8);

  std::vector<std::pair<int, std::string>> entries = {
      {23, "Faza"},
      {11, "Dinda"},
      {12, "Nurfaizi"},
      {1, "Salshabila"},
  };

  for (const auto &[k, v] : entries) {
    EXPECT_TRUE(ht.insert(k, v));
  }

  for (const auto &[k, v] : entries) {
    auto *val = ht.lookup(k);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, v);
  }
}

TEST_F(CuckooHashTableTest, UpdateExistingKey) {
  CuckooHashTable<int, std::string> ht(8);

  ht.insert(100, "Initial Value");
  auto *val1 = ht.lookup(100);
  ASSERT_NE(val1, nullptr);
  EXPECT_EQ(*val1, "Initial Value");

  ht.insert(100, "Updated Value");
  auto *val2 = ht.lookup(100);
  ASSERT_NE(val2, nullptr);
  EXPECT_EQ(*val2, "Updated Value");
}

TEST_F(CuckooHashTableTest, MutateReturnedPointerDirectly) {
  CuckooHashTable<int, int> ht(8);

  ht.insert(1, 100);
  int *val = ht.lookup(1);
  ASSERT_NE(val, nullptr);

  // In-place mutation via returned pointer
  *val = 99;

  int *updated_value = ht.lookup(1);
  ASSERT_NE(updated_value, nullptr);
  ASSERT_EQ(*updated_value, 99);
}

TEST_F(CuckooHashTableTest, RemoveNonExistentKey) {
  CuckooHashTable<int, int> ht(8);

  EXPECT_FALSE(ht.remove(0));
}

TEST_F(CuckooHashTableTest, RemoveExistingKey) {
  CuckooHashTable<int, std::string> ht(8);

  ht.insert(10, "Alpha");
  ht.insert(20, "Beta");

  EXPECT_TRUE(ht.remove(10));
  EXPECT_EQ(ht.lookup(10), nullptr);

  // Beta should still exist
  auto *val = ht.lookup(20);
  ASSERT_NE(val, nullptr);
  EXPECT_EQ(*val, "Beta");
}

TEST_F(CuckooHashTableTest, ReInsertAfterRemoval) {
  CuckooHashTable<int, std::string> ht(8);

  ht.insert(10, "V1");
  ht.remove(10);
  EXPECT_EQ(ht.lookup(10), nullptr);

  ht.insert(10, "V2");
  auto *val = ht.lookup(10);
  ASSERT_NE(val, nullptr);
  EXPECT_EQ(*val, "V2");
}

TEST_F(CuckooHashTableTest, TriggerEvictionChainWithoutRehash) {
  // Small capacity to intentionally force cuckoo displacement
  CuckooHashTable<int, int> ht(4);

  // Insert items that will kick each other between table1 and table2
  for (int i = 1; i <= 3; ++i) {
    EXPECT_TRUE(ht.insert(i, i * 10));
  }

  for (int i = 1; i <= 3; ++i) {
    auto *val = ht.lookup(i);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, i * 10);
  }
}

TEST_F(CuckooHashTableTest, TriggerRehashAndCapacityGrowth) {
  // Start with a minimal capacity of 2 (total 4 slots)
  CuckooHashTable<int, int> ht(2);

  // Insert 20 elements to force multiple rehash/growth steps
  for (int i = 0; i < 20; ++i) {
    EXPECT_TRUE(ht.insert(i, i * 100));
  }

  // Verify all 20 elements survived rehashing
  for (int i = 0; i < 20; ++i) {
    auto *val = ht.lookup(i);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, i * 100);
  }
}

TEST_F(CuckooHashTableTest, StringKeysAndComplexValues) {
  CuckooHashTable<std::string, std::vector<int>> ht(4);

  std::vector<int> vec1 = {1, 2, 3};
  std::vector<int> vec2 = {4, 5, 6};

  ht.insert("group_a", vec1);
  ht.insert("group_b", vec2);

  auto *res = ht.lookup("group_a");
  ASSERT_NE(res, nullptr);
  EXPECT_EQ(*res, vec1);
}

TEST_F(CuckooHashTableTest, NegativeAndZeroKeys) {
  CuckooHashTable<int, std::string> ht(8);

  ht.insert(0, "Zero");
  ht.insert(-50, "NegativeFifty");
  ht.insert(-1, "NegativeOne");

  auto *v0 = ht.lookup(0);
  ASSERT_NE(v0, nullptr);
  EXPECT_EQ(*v0, "Zero");

  auto *v50 = ht.lookup(-50);
  ASSERT_NE(v50, nullptr);
  EXPECT_EQ(*v50, "NegativeFifty");

  auto *v1 = ht.lookup(-1);
  ASSERT_NE(v1, nullptr);
  EXPECT_EQ(*v1, "NegativeOne");
}

} // namespace test

} // namespace turtle
