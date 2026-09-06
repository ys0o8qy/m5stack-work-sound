#pragma once
#include "toy_config.h"
#include <cstddef>
#include <cstdint>

namespace toy {
// No Arduino dependencies: this timing/recognition boundary is tested on the host.
class InputSession {
 public:
  void touch(uint32_t now) { active_ = true; last_ = now; }
  void held(uint32_t now) { if (active_) last_ = now; }
  void append(char value, uint32_t now) {
    touch(now);
    if (length_ == kMaxInput) { overflow_ = true; return; }
    if (value >= 'A' && value <= 'Z') value += 'a' - 'A';
    text_[length_++] = value;
    text_[length_] = '\0';
  }
  void backspace(uint32_t now) {
    touch(now);
    // Once overflowed, never recognize a truncated prefix this round.
    if (length_) text_[--length_] = '\0';
  }
  bool due(uint32_t now, uint32_t idle_ms) const {
    return active_ && static_cast<uint32_t>(now - last_) >= idle_ms;
  }
  bool isWordCandidate() const {
    if (!length_ || overflow_) return false;
    for (size_t i = 0; i < length_; ++i)
      if (text_[i] < 'a' || text_[i] > 'z') return false;
    return true;
  }
  const char* text() const { return text_; }
  bool active() const { return active_; }
  void clear() { length_ = 0; text_[0] = '\0'; overflow_ = false; active_ = false; }
 private:
  char text_[kMaxInput + 1] = {};
  size_t length_ = 0;
  uint32_t last_ = 0;
  bool overflow_ = false;
  bool active_ = false;
};
}

