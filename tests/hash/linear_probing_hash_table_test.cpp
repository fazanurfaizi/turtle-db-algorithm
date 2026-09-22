#include "turtledb/hash/linear_probing_hash_table.h"
#include <cstddef>
#include <functional>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

namespace turtle {

namespace test {

class LinearProbingHashTest : public ::testing::Test {};

TEST_F(LinearProbingHashTest, InsertAndGet) {
  LinearProbingHash<int, int> map(10);
  map.insert(1, 100);
  map.insert(2, 200);

  EXPECT_EQ(map.get(1), 100);
  EXPECT_EQ(map.get(2), 200);
}

TEST_F(LinearProbingHashTest, UpdateExistingKey) {
  LinearProbingHash<int, std::string> map(10);
  map.insert(1, "Init");
  EXPECT_EQ(map.get(1), "Init");

  // Overwrite existing key value
  map.insert(1, "Updated");
  EXPECT_EQ(map.get(1), "Updated");
}

TEST_F(LinearProbingHashTest, GetNontExistentKey) {
  LinearProbingHash<int, int> map(10);
  map.insert(1, 100);

  EXPECT_EQ(map.get(999), std::nullopt);
}

TEST_F(LinearProbingHashTest, RemoveExistingKey) {
  LinearProbingHash<int, int> map(10);
  map.insert(1, 100);
  map.insert(2, 200);

  EXPECT_TRUE(map.remove(1));
  EXPECT_EQ(map.get(1), std::nullopt);
  EXPECT_EQ(map.get(2), 200);
}

TEST_F(LinearProbingHashTest, CollisionResolution) {
  LinearProbingHash<int, int> map(5);

  map.insert(0, 10);
  map.insert(5, 20);
  map.insert(10, 30);

  EXPECT_EQ(map.get(0), 10);
  EXPECT_EQ(map.get(5), 20);
  EXPECT_EQ(map.get(10), 30);
}

TEST_F(LinearProbingHashTest, TombstoneRecyclingOnInsert) {
  LinearProbingHash<int, int> map(10);
  map.insert(1, 100);
  map.insert(11, 200);

  // remove key 1 to leave a DELETED slot/tombstone
  EXPECT_TRUE(map.remove(1));

  // Insert key 21, which should reuse the tombstone slot
  map.insert(21, 300);
  EXPECT_EQ(map.get(21), 300);
  EXPECT_EQ(map.get(11), 200);
}

TEST_F(LinearProbingHashTest, ProbingAcrossDeletedSlots) {
  LinearProbingHash<int, int> map(10);
  map.insert(1, 100);
  map.insert(11, 200);
  map.insert(21, 300);

  // Delete middle key in probe chain
  map.remove(11);

  EXPECT_EQ(map.get(21), 300);
}

TEST_F(LinearProbingHashTest, AutomaticRehashTrigger) {
  LinearProbingHash<int, int> map(4);

  map.insert(1, 10);
  map.insert(2, 20);
  map.insert(3, 30);

  EXPECT_EQ(map.get(1), 10);
  EXPECT_EQ(map.get(2), 20);
  EXPECT_EQ(map.get(3), 30);
}

TEST_F(LinearProbingHashTest, StressInsertRehash) {
  LinearProbingHash<int, int> map(4);
  const int count = 500;

  for (size_t i = 0; i < count; ++i) {
    map.insert(i, i * 10);
  }

  for (size_t i = 0; i < count; ++i) {
    EXPECT_EQ(map.get(i), i * 10);
  }
}

TEST_F(LinearProbingHashTest, StringKeysAndValues) {
  LinearProbingHash<std::string, std::string> map(10);

  map.insert("user_a", "admin");
  map.insert("user_b", "guest");

  EXPECT_EQ(map.get("user_a"), "admin");
  EXPECT_EQ(map.get("user_b"), "guest");
}

struct CustomKey {
  int id;
  std::string name;

  bool operator==(const CustomKey &other) const {
    return id == other.id && name == other.name;
  }
};

} // namespace test
} // namespace turtle

namespace std {
template <> struct hash<turtle::test::CustomKey> {
  size_t operator()(const turtle::test::CustomKey &k) const {
    return std::hash<int>()(k.id) ^ (std::hash<std::string>()(k.name) << 1);
  }
};
} // namespace std

namespace turtle {
namespace test {

TEST_F(LinearProbingHashTest, CustomStructKey) {
  LinearProbingHash<CustomKey, int> map(10);
  CustomKey k1{10, "Alpha"};
  CustomKey k2{20, "Beta"};

  map.insert(k1, 450);
  map.insert(k2, 850);

  EXPECT_EQ(map.get(k1), 450);
  EXPECT_EQ(map.get(k2), 850);
}

TEST_F(LinearProbingHashTest, ZeroAndNegativeKeys) {
  LinearProbingHash<int, int> map(10);

  map.insert(0, 1000);
  map.insert(-1, 2000);
  map.insert(-9999, 3000);

  EXPECT_EQ(map.get(0), 1000);
  EXPECT_EQ(map.get(-1), 2000);
  EXPECT_EQ(map.get(-9999), 3000);
}

} // namespace test
} // namespace turtle
