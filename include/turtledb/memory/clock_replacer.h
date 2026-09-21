#pragma once

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>
namespace turtle {

class ClockReplacer {
public:
  explicit ClockReplacer(int frames)
      : frames_(frames), pages_(frames, kEmpty), second_chance_(frames, false),
        hand_(0), page_faults_(0), hits_(0) {
    if (frames <= 0) {
      throw std::invalid_argument("frame count must be positive");
    }
  }

  // Accesses one page. Returns true on a hit, false on a fault.
  bool access(int page) {
    if (this->find_and_update(page)) {
      ++this->hits_;
      return true;
    }
    this->replace_and_update(page);
    ++this->page_faults_;
    return false;
  }

  // Runs a whole reference string, e.g. "0 4 1 4 2".
  void run(const std::string &reference_string) {
    std::istringstream in(reference_string);
    int page;
    while (in >> page) {
      this->access(page);
    }
  }

  void reset() {
    std::fill(this->pages_.begin(), this->pages_.end(), this->kEmpty);
    std::fill(this->second_chance_.begin(), this->second_chance_.end(), false);
    this->hand_ = 0;
    this->page_faults_ = 0;
    this->hits_ = 0;
  }

  int page_faults() const { return this->page_faults_; }

  int hits() const { return this->hits_; }

  int frames() const { return this->frames_; }

  const std::vector<int> &pages() const { return this->pages_; }

private:
  static constexpr int kEmpty = -1;

  int frames_;
  std::vector<int> pages_;
  std::vector<bool> second_chance_;
  int hand_;
  int page_faults_;
  int hits_;

  // If the page is resident, give it a second change and report a hit.
  bool find_and_update(int page) {
    for (int i = 0; i < this->frames_; ++i) {
      if (this->pages_[i] == page) {
        this->second_chance_[i] = true;
        return true;
      }
    }
    return false;
  }

  // Sweeps the hand until it finds a frame without a second change,
  // clearing the reference bit of everything it passes.
  void replace_and_update(int page) {
    while (this->second_chance_[this->hand_]) {
      this->second_chance_[this->hand_] = false;
      this->hand_ = (this->hand_ + 1) % this->frames_;
    }
    this->pages_[this->hand_] = page;
    this->hand_ = (this->hand_ + 1) % this->frames_;
  }
};

} // namespace turtle
