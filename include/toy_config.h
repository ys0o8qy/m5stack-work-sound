#pragma once
#include <cstdint>

namespace toy {
// Defaults on first boot. Parent settings on the device override these in NVS.
constexpr uint32_t kDefaultIdleMs = 3000;
constexpr uint32_t kMinIdleMs = 1000;
constexpr uint32_t kMaxIdleMs = 15000;
constexpr uint8_t kDefaultVolume = 80;  // Audible midrange effects; speech has separate attenuation.
constexpr uint8_t kSpeechChannelVolume = 204;
constexpr uint8_t kMaxVolume = 160;
constexpr uint8_t kDefaultBrightness = 130;
constexpr uint32_t kParentHoldMs = 2000;
constexpr uint32_t kSoundSpacingMs = 65;
constexpr uint32_t kFrameMs = 33;
constexpr unsigned kMaxInput = 24;
}
