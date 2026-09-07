#pragma once
#include <M5GFX.h>
#include "toy_config.h"

namespace toy {
class Visuals {
 public:
  explicit Visuals(M5Canvas& canvas) : c_(canvas) {}
  void burst(uint16_t key, uint32_t now);
  void setInput(const char* text, uint32_t now, bool deleted = false);
  void clearInput() { inputText_[0] = '\0'; activeKey_ = 0; deletedChar_ = 0; }
  void showWord(const char* word, const char* illustration, uint32_t now);
  void clearWord() { word_ = nullptr; }
  void draw(uint32_t now, bool book = false, unsigned page = 0, unsigned pages = 0);
 private:
  struct Particle {
    float x = 0, y = 0, vx = 0, vy = 0, r = 0;
    uint32_t born = 0;
    uint16_t color = 0;
    uint8_t kind = 0;
    bool alive = false;
  } particles_[40];
  M5Canvas& c_;
  unsigned next_ = 0;
  uint32_t previous_ = 0, wordBorn_ = 0, burstBorn_ = 0;
  const char* word_ = nullptr;
  const char* wordIcon_ = nullptr;
  float wordTime_ = 0;
  uint16_t activeKey_ = 0;
  char inputText_[kMaxInput + 1] = {};
  char deletedChar_ = 0;
  unsigned deletedColumn_ = 0;
  uint32_t editBorn_ = 0;
  void star(int x, int y, int r, uint16_t color, float rotation = 0);
  void face(int x, int y, int spread = 10, bool blink = false);
  void illustration(const char* word, int x, int y);
  bool extendedIllustration(const char* word, int x, int y);
  void drawKey(uint32_t now);
  void drawInput(uint32_t now);
};
}
