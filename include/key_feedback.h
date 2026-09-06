#pragma once
#include <cstdint>

namespace toy {
constexpr uint16_t kKeyFn = 256;
constexpr uint16_t kKeyShift = 257;
constexpr uint16_t kKeyCtrl = 258;
constexpr uint16_t kKeyOpt = 259;
constexpr uint16_t kKeyAlt = 260;

struct KeyStyle {
  uint8_t color;
  uint8_t animation;
  uint8_t effect;
};

constexpr uint16_t normalizeKey(uint16_t key) {
  return key >= 'A' && key <= 'Z' ? key + ('a' - 'A') : key;
}

inline KeyStyle styleForKey(uint16_t key) {
  key = normalizeKey(key);
  const unsigned index = key >= 'a' && key <= 'z' ? key - 'a' : key + 26;
  // Identity is stable; only particle trajectories have random variation.
  // Letters cycle through bubbles, balls, flowers and stars.
  const uint8_t animation = index % 4;
  return {uint8_t((index * 5 + 1) % 6), animation,
          uint8_t(animation * 5 + (index / 4) % 5)};
}
}
