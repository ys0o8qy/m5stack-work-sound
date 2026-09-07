#pragma once
#include "toy_config.h"

namespace toy {
struct LegacySettings {
  uint32_t idleMs;
  uint8_t volume;
  uint8_t brightness;
};
struct Settings {
  uint32_t idleMs = kDefaultIdleMs;
  uint8_t volume = kDefaultVolume;
  uint8_t brightness = kDefaultBrightness;
  uint32_t dimMs = 60000;
};
static_assert(sizeof(LegacySettings) == 8, "Preserve the original NVS layout");
static_assert(sizeof(Settings) == 12, "Versioned settings layout must remain stable");
inline Settings migrateSettings(const LegacySettings& old) {
  Settings result;
  result.idleMs = old.idleMs;
  result.volume = old.volume;
  result.brightness = old.brightness;
  return result;
}
inline bool validDimDelay(uint32_t value) {
  return value == 0 || value == 30000 || value == 60000 || value == 120000;
}
}
