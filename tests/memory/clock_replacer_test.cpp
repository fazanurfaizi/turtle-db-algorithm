#include "turtledb/memory/clock_replacer.h"
#include "gtest/gtest.h"
#include <stdexcept>

const int CACHE_CAPACITY = 3;

TEST(ClockReplacerTest, InvalidFrameCountThrows) {
  EXPECT_THROW(turtle::ClockReplacer(0), std::invalid_argument);
  EXPECT_THROW(turtle::ClockReplacer(-1), std::length_error);
}

TEST(ClockReplacerTest, InitialStateIsEmpty) {
  turtle::ClockReplacer replacer(CACHE_CAPACITY);

  EXPECT_EQ(replacer.frames(), CACHE_CAPACITY);
  EXPECT_EQ(replacer.page_faults(), 0);
  EXPECT_EQ(replacer.hits(), 0);

  const auto &pages = replacer.pages();
  for (int page : pages) {
    EXPECT_EQ(page, -1);
  }
}

TEST(ClockReplacerTest, PageAccessHitsAndFaults) {
  turtle::ClockReplacer replacer(CACHE_CAPACITY);

  // Accessing a new page causes a page fault (returns false)
  EXPECT_FALSE(replacer.access(1));
  EXPECT_EQ(replacer.page_faults(), 1);
  EXPECT_EQ(replacer.hits(), 0);

  // Accessing an existing page causes a hit (returns true)
  EXPECT_TRUE(replacer.access(1));
  EXPECT_EQ(replacer.page_faults(), 1);
  EXPECT_EQ(replacer.hits(), 1);
}

TEST(ClockReplacerTest, SimpleEvictionSweep) {
  turtle::ClockReplacer replacer(CACHE_CAPACITY);

  // Fill all frames: pages [1, 2, 3] loaded sequentially
  EXPECT_FALSE(replacer.access(1));
  EXPECT_FALSE(replacer.access(2));
  EXPECT_FALSE(replacer.access(3));

  // Access page 2 to set its second-change bit
  EXPECT_TRUE(replacer.access(2));

  // Access page 4 (fault) -> Clock hand start at index 0 (page 1, bit=0).
  // Page 1 is evicted and replaced with page 4.
  EXPECT_FALSE(replacer.access(4));

  const auto &pages = replacer.pages();
  EXPECT_EQ(pages[0], 4);
  EXPECT_EQ(pages[1], 2);
  EXPECT_EQ(pages[2], 3);
}

TEST(ClockReplacerTest, RespectsSecondChanceBit) {
  turtle::ClockReplacer replacer(3);

  // Reference sequence: 0, 4, 1, 4, 2
  replacer.run("0 4 1 4 2");

  // State after sequence:
  // Frame 0 holds page 2 (evicted page 0 because page 4 had second_chance set)
  // Frame 1 holds page 4
  // Frame 2 holds page 1
  const auto &pages = replacer.pages();
  EXPECT_EQ(pages[0], 2);
  EXPECT_EQ(pages[1], 4);
  EXPECT_EQ(pages[2], 1);

  EXPECT_EQ(replacer.hits(), 1);        // Page 4 hit
  EXPECT_EQ(replacer.page_faults(), 4); // Pages 0, 4, 1, 2 faulted
}

TEST(ClockReplacerTest, ResetClearsAllFramesAndStats) {
  turtle::ClockReplacer replacer(CACHE_CAPACITY);

  replacer.run("0 4 1 4 2 4 3");
  EXPECT_GT(replacer.page_faults(), 0);
  EXPECT_GT(replacer.hits(), 0);

  replacer.reset();

  EXPECT_EQ(replacer.page_faults(), 0);
  EXPECT_EQ(replacer.hits(), 0);

  for (int page : replacer.pages()) {
    EXPECT_EQ(page, -1);
  }
}

TEST(ClockReplacerTest, FullReferenceStringTrace) {
  turtle::ClockReplacer replacer(3);

  // Complete reference trace run
  replacer.run("0 4 1 4 2 4 3 4 2 4 0 4 1 4 2 4 3 4");

  // Expected final frame state: pages [3, 4, 2]
  const auto &pages = replacer.pages();
  EXPECT_EQ(pages[0], 3);
  EXPECT_EQ(pages[1], 4);
  EXPECT_EQ(pages[2], 2);

  EXPECT_EQ(replacer.hits(), 9);
  EXPECT_EQ(replacer.page_faults(), 9);
}
