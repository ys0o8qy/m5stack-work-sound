#pragma once
#include <cstddef>
#include <cstdint>
namespace toy {
struct Clip { const int16_t* data; size_t samples; };
struct WordAudio { const char* word; Clip clip; };
extern const WordAudio kWords[];
extern const size_t kWordCount;
extern const Clip kEffects[20];
}
