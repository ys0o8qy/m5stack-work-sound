#pragma once
#include <cstdint>

namespace toy {
// Visual inactivity only; never changes input timing or swallows a wake key.
class IdleDimmer {
 public:
  void touch(uint32_t now) { lastActivity_ = now; }
  uint8_t brightness(uint32_t now, uint32_t delay, uint8_t normal) const {
    const uint32_t idle = uint32_t(now - lastActivity_);
    if (!delay || idle <= delay || normal <= 20) return normal;
    const uint32_t fade = idle - delay;
    if (fade >= 1500) return 20;
    return normal - (uint32_t(normal - 20) * fade / 1500);
  }
 private:
  uint32_t lastActivity_ = 0;
};
}
