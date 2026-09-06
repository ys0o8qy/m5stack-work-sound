#include "toy_visuals.h"
#include "key_feedback.h"
#include <Arduino.h>
#include <cmath>
#include <cstring>

namespace toy {
namespace {
constexpr uint16_t BG = 0x0926, INK = 0x1148, WHITE = 0xffbd;
constexpr uint16_t PEACH = 0xfd52, MINT = 0x7ef6, GOLD = 0xff09;
constexpr uint16_t BLUE = 0x6d9f, PINK = 0xfc16, LILAC = 0xb4ff;
constexpr uint16_t colors[] = {PEACH, MINT, GOLD, BLUE, PINK, LILAC};
bool is(const char* a, const char* b) { return std::strcmp(a, b) == 0; }
}

void Visuals::star(int x, int y, int r, uint16_t color, float rotation) {
  for (int i = 0; i < 10; ++i) {
    float a = rotation - 1.5708f + i * .628319f;
    float b = a + .628319f;
    float ra = i % 2 ? r * .45f : r;
    float rb = i % 2 ? r : r * .45f;
    c_.fillTriangle(x, y, x + cosf(a) * ra, y + sinf(a) * ra,
                    x + cosf(b) * rb, y + sinf(b) * rb, color);
  }
}

void Visuals::face(int x, int y, int spread, bool blink) {
  if (blink) {
    c_.drawFastHLine(x - spread - 2, y, 5, INK);
    c_.drawFastHLine(x + spread - 2, y, 5, INK);
  } else {
    c_.fillCircle(x - spread, y, 2, INK);
    c_.fillCircle(x + spread, y, 2, INK);
  }
  c_.drawLine(x - 3, y + 8, x, y + 10, INK);
  c_.drawLine(x, y + 10, x + 3, y + 8, INK);
}

void Visuals::burst(uint16_t key, uint32_t now) {
  word_ = nullptr;
  activeKey_ = normalizeKey(key);
  burstBorn_ = now;
  int centerX = 120 + random(-12, 13), centerY = 59;
  const auto style = styleForKey(key);
  for (int i = 0; i < 8; ++i) {
    auto& p = particles_[next_++ % 40];
    float angle = i * .785398f + random(100) * .01f;
    p.x = centerX; p.y = centerY;
    p.vx = cosf(angle) * random(25, 80);
    p.vy = sinf(angle) * random(20, 65) - 20;
    p.r = random(5, 12);
    p.born = now; p.color = colors[style.color]; p.kind = style.animation; p.alive = true;
  }
}

void Visuals::setInput(const char* text, uint32_t now, bool deleted) {
  size_t before = strlen(inputText_), after = strlen(text);
  deletedChar_ = deleted && before > after ? inputText_[before - 1] : 0;
  deletedColumn_ = before ? (before > 12 ? 11 : before - 1) : 0;
  editBorn_ = now;
  strncpy(inputText_, text, kMaxInput);
  inputText_[kMaxInput] = '\0';
}

void Visuals::drawKey(uint32_t now) {
  const auto style = styleForKey(activeKey_);
  float age = uint32_t(now - burstBorn_) / 1000.f;
  float pop = 1.f - .35f * expf(-age * 7) * cosf(age * 12);
  float bump = sinf(age * 2.5f) * 2;
  if (style.animation == 0) bump = -8 * (1 - expf(-age * 3)) + sinf(age * 3) * 3;
  if (style.animation == 1) bump = -22 * fabsf(sinf(age * 7)) * expf(-age * 1.3f);
  int x = 120, y = 62 + bump, r = 29 * pop;
  uint16_t color = colors[style.color];
  c_.fillEllipse(x, 99, 23 + bump / 3, 3, 0x1949);
  if (style.animation == 3) {
    star(x, y, r + 9, color, .16f * sinf(age * 3));
    for (int i = 0; i < 3; ++i) {
      float a = age * 1.7f + i * 2.0944f;
      star(x + cosf(a) * 49, y + sinf(a) * 27, 4, color, a);
    }
  }
  else if (style.animation == 2) {
    for (int i = 0; i < 5; ++i) {
      float a = i * 1.256637f - 1.5708f + .20f * sinf(age * 2);
      float bloom = .35f + .4f * (1 - expf(-age * 4));
      c_.fillCircle(x + cosf(a) * r * bloom, y + sinf(a) * r * bloom, r * .52f, color);
    }
    c_.fillCircle(x, y, r * .7f, color);
  } else {
    float squash = style.animation == 1 ? .12f * expf(-age * 1.3f) * cosf(age * 14) : 0;
    c_.fillEllipse(x, y, r * (1 + squash), r * (1 - squash), color);
    if (style.animation == 0) {
      c_.drawCircle(x, y, r + 4, color);
      c_.fillEllipse(x - r / 2, y - r / 2, 3, 6, WHITE);
      if (age < .75f) c_.drawEllipse(x, 98, 28 + age * 35, 3 + age * 4, color);
    }
  }
  c_.setTextColor(INK); c_.setTextDatum(middle_center); c_.setTextFont(4);
  if (activeKey_ >= 33 && activeKey_ <= 126) {
    char glyph[] = {char(activeKey_ >= 'a' && activeKey_ <= 'z' ? activeKey_ - 32 : activeKey_), 0};
    c_.setTextSize(1.8f * pop); c_.drawString(glyph, x, y);
  } else if (activeKey_ == '\b') {
    c_.drawWideLine(x - 18, y, x + 16, y, 4, INK);
    c_.drawWideLine(x - 18, y, x - 7, y - 11, 4, INK);
    c_.drawWideLine(x - 18, y, x - 7, y + 11, 4, INK);
  } else if (activeKey_ == ' ') {
    c_.drawWideLine(x - 17, y + 5, x + 17, y + 5, 4, INK);
    c_.drawWideLine(x - 17, y - 5, x - 17, y + 5, 4, INK);
    c_.drawWideLine(x + 17, y - 5, x + 17, y + 5, 4, INK);
  } else if (activeKey_ == '\r' || activeKey_ == '\t') {
    c_.drawWideLine(x + 15, y - 12, x + 15, y + 5, 4, INK);
    c_.drawWideLine(x - 15, y + 5, x + 15, y + 5, 4, INK);
    c_.drawWideLine(x - 15, y + 5, x - 5, y - 5, 4, INK);
    c_.drawWideLine(x - 15, y + 5, x - 5, y + 15, 4, INK);
  } else {
    const char* label = activeKey_ == kKeyFn ? "fn" : activeKey_ == kKeyShift ? "Aa" :
                        activeKey_ == kKeyCtrl ? "ctrl" : activeKey_ == kKeyAlt ? "alt" : "opt";
    c_.setTextSize(pop); c_.drawString(label, x, y);
  }
  c_.setTextSize(1);
}

void Visuals::drawInput(uint32_t now) {
  c_.fillRect(0, 106, 240, 29, 0x1168);
  c_.drawFastHLine(8, 106, 224, 0x2a8c);
  size_t length = strlen(inputText_);
  size_t start = length > 12 ? length - 12 : 0;
  c_.setTextDatum(top_left); c_.setTextFont(1); c_.setTextSize(3);
  for (size_t i = start; i < length; ++i) {
    int x = 12 + (i - start) * 18;
    char glyph[] = {inputText_[i], 0};
    c_.setTextColor(colors[styleForKey(uint8_t(inputText_[i])).color]);
    if (inputText_[i] == ' ') c_.drawFastHLine(x + 2, 128, 12, 0x7bf0);
    else c_.drawString(glyph, x, 110);
  }
  if (start) c_.fillTriangle(2, 120, 7, 116, 7, 124, 0x9cf5);
  if (deletedChar_ && uint32_t(now - editBorn_) < 250) {
    float shrink = 1.f - uint32_t(now - editBorn_) / 250.f;
    char glyph[] = {deletedChar_, 0};
    c_.setTextDatum(middle_center); c_.setTextColor(0x7bf0); c_.setTextSize(3 * shrink);
    c_.drawString(glyph, 21 + deletedColumn_ * 18, 121 - (1 - shrink) * 8);
  }
  c_.setTextSize(1);
  if (!length && !deletedChar_) {
    c_.fillCircle(113, 121, 1, 0x5390); c_.fillCircle(120, 121, 1, 0x5390); c_.fillCircle(127, 121, 1, 0x5390);
  }
}

void Visuals::showWord(const char* word, const char* icon, uint32_t now) {
  word_ = word; wordIcon_ = icon; wordBorn_ = now;
  for (auto& p : particles_) p.alive = false;
}

void Visuals::illustration(const char* w, int x, int y) {
  if (extendedIllustration(w, x, y)) return;
  if (is(w, "cat") || is(w, "dog") || is(w, "bear") || is(w, "bunny") || is(w, "cow") || is(w, "pig")) {
    uint16_t coat = is(w, "pig") ? PINK : is(w, "cow") ? WHITE : is(w, "bear") ? PEACH : GOLD;
    if (is(w, "cat")) {
      c_.fillTriangle(x - 27, y - 8, x - 25, y - 34, x - 6, y - 21, coat);
      c_.fillTriangle(x + 27, y - 8, x + 25, y - 34, x + 6, y - 21, coat);
    } else if (is(w, "bunny")) {
      c_.fillRoundRect(x - 21, y - 44, 14, 39, 7, coat);
      c_.fillRoundRect(x + 7, y - 44, 14, 39, 7, coat);
      c_.fillRoundRect(x - 17, y - 38, 6, 26, 3, PEACH);
      c_.fillRoundRect(x + 11, y - 38, 6, 26, 3, PEACH);
    } else {
      c_.fillEllipse(x - 25, y - 16, 12, is(w, "dog") ? 23 : 12, coat);
      c_.fillEllipse(x + 25, y - 16, 12, is(w, "dog") ? 23 : 12, coat);
    }
    c_.fillEllipse(x, y, 29, 25, coat);
    if (is(w, "cow")) c_.fillCircle(x - 16, y - 7, 11, INK);
    face(x, y - 3);
    if (is(w, "pig") || is(w, "cow")) {
      c_.fillEllipse(x, y + 12, 13, 8, PEACH);
      c_.fillCircle(x - 5, y + 12, 2, INK); c_.fillCircle(x + 5, y + 12, 2, INK);
    } else {
      c_.fillTriangle(x - 4, y + 5, x + 4, y + 5, x, y + 9, INK);
    }
    if (is(w, "cat")) {
      for (int i = -1; i <= 1; ++i) {
        c_.drawLine(x - 15, y + 9, x - 38, y + 9 + i * 6, WHITE);
        c_.drawLine(x + 15, y + 9, x + 38, y + 9 + i * 6, WHITE);
      }
    }
  } else if (is(w, "fish")) {
    c_.fillTriangle(x - 15, y, x - 43, y - 19, x - 43, y + 19, PEACH);
    c_.fillEllipse(x + 1, y, 29, 20, BLUE);
    c_.fillTriangle(x - 5, y - 16, x + 9, y - 29, x + 17, y - 12, BLUE);
    c_.fillCircle(x + 16, y - 5, 5, WHITE); c_.fillCircle(x + 18, y - 5, 2, INK);
    c_.drawCircle(x + 40, y - 15, 4, MINT); c_.drawCircle(x + 47, y - 28, 3, MINT);
  } else if (is(w, "bird") || is(w, "duck")) {
    c_.fillEllipse(x - 5, y + 6, 27, 20, is(w, "duck") ? GOLD : BLUE);
    c_.fillCircle(x + 15, y - 12, 18, is(w, "duck") ? GOLD : BLUE);
    c_.fillEllipse(x - 9, y + 4, 16, 10, PEACH);
    c_.fillTriangle(x + 28, y - 14, x + 43, y - 8, x + 27, y - 3, PEACH);
    c_.fillCircle(x + 19, y - 16, 3, INK);
    c_.drawFastHLine(x - 9, y + 29, 14, GOLD); c_.drawFastHLine(x + 8, y + 29, 14, GOLD);
  } else if (is(w, "frog")) {
    c_.fillEllipse(x, y + 5, 31, 23, MINT);
    for (int s : {-1, 1}) {
      c_.fillCircle(x + s * 18, y - 16, 12, MINT);
      c_.fillCircle(x + s * 18, y - 17, 7, WHITE);
      c_.fillCircle(x + s * 18, y - 17, 3, INK);
    }
    c_.drawLine(x - 13, y + 6, x, y + 14, INK); c_.drawLine(x, y + 14, x + 13, y + 6, INK);
  } else if (is(w, "bee")) {
    c_.fillEllipse(x - 10, y - 21, 15, 16, WHITE); c_.fillEllipse(x + 13, y - 21, 15, 16, WHITE);
    c_.fillEllipse(x, y + 1, 30, 20, GOLD);
    c_.fillRect(x - 12, y - 16, 6, 35, INK); c_.fillRect(x, y - 18, 6, 39, INK);
    c_.fillCircle(x + 19, y - 3, 3, INK);
  } else if (is(w, "apple")) {
    c_.drawWideLine(x, y - 16, x + 4, y - 34, 4, PEACH);
    c_.fillEllipse(x + 12, y - 29, 11, 6, MINT);
    c_.fillEllipse(x - 12, y + 1, 21, 25, PINK); c_.fillEllipse(x + 12, y + 1, 21, 25, PINK);
    c_.fillEllipse(x - 21, y - 8, 3, 8, WHITE);
  } else if (is(w, "banana")) {
    c_.fillEllipse(x, y - 7, 35, 34, GOLD);
    c_.fillEllipse(x + 10, y - 22, 34, 30, BG);
    c_.fillCircle(x - 26, y - 22, 4, PEACH);
  } else if (is(w, "milk")) {
    c_.fillRect(x - 22, y - 20, 44, 49, WHITE);
    c_.fillTriangle(x - 22, y - 20, x - 9, y - 34, x + 22, y - 20, BLUE);
    c_.fillRect(x - 9, y - 34, 30, 14, BLUE);
    c_.fillRect(x - 22, y, 44, 17, BLUE);
    c_.fillCircle(x, y + 8, 5, WHITE);
  } else if (is(w, "egg")) {
    c_.fillEllipse(x, y, 25, 33, WHITE); c_.fillCircle(x, y + 6, 14, GOLD);
  } else if (is(w, "cake")) {
    c_.fillRoundRect(x - 33, y - 4, 66, 31, 7, PEACH);
    c_.fillRoundRect(x - 33, y - 9, 66, 13, 6, PINK);
    c_.drawFastHLine(x - 39, y + 29, 78, WHITE);
    c_.fillRect(x - 2, y - 27, 5, 18, MINT); c_.fillEllipse(x, y - 33, 4, 7, GOLD);
  } else if (is(w, "car") || is(w, "bus")) {
    int width = is(w, "bus") ? 76 : 70;
    c_.fillRoundRect(x - width / 2, y - 17, width, 40, 8, GOLD);
    c_.fillRoundRect(x - 25, y - 12, 23, 17, 3, BLUE);
    c_.fillRoundRect(x + 4, y - 12, 23, 17, 3, BLUE);
    for (int s : {-1, 1}) { c_.fillCircle(x + s * 23, y + 24, 10, INK); c_.fillCircle(x + s * 23, y + 24, 5, WHITE); }
    if (is(w, "bus")) c_.fillRect(x - 35, y - 23, 70, 6, GOLD);
  } else if (is(w, "boat")) {
    c_.fillTriangle(x - 39, y + 8, x + 39, y + 8, x + 22, y + 28, PEACH);
    c_.fillTriangle(x - 39, y + 8, x - 22, y + 28, x + 22, y + 28, PEACH);
    c_.drawFastVLine(x, y - 35, 44, WHITE);
    c_.fillTriangle(x - 4, y - 32, x - 4, y + 2, x - 31, y + 2, MINT);
    c_.fillTriangle(x + 4, y - 25, x + 4, y + 2, x + 27, y + 2, GOLD);
    c_.drawFastHLine(x - 47, y + 32, 94, BLUE);
  } else if (is(w, "moon")) {
    c_.fillCircle(x, y, 29, GOLD); c_.fillCircle(x + 16, y - 13, 25, BG); star(x + 29, y + 15, 7, WHITE);
  } else if (is(w, "star")) {
    star(x, y, 35, GOLD); face(x, y - 1, 8);
  } else if (is(w, "sun")) {
    for (int i = 0; i < 8; ++i) {
      float a = i * .785398f;
      c_.drawWideLine(x + cosf(a) * 29, y + sinf(a) * 29, x + cosf(a) * 38, y + sinf(a) * 38, 3, GOLD);
    }
    c_.fillCircle(x, y, 23, GOLD); face(x, y - 3, 8);
  } else if (is(w, "flower")) {
    c_.fillRect(x - 2, y, 4, 34, MINT); c_.fillEllipse(x + 10, y + 20, 10, 5, MINT);
    for (int i = 0; i < 5; ++i) { float a = i * 1.256637f; c_.fillCircle(x + cosf(a) * 17, y - 9 + sinf(a) * 17, 13, PINK); }
    c_.fillCircle(x, y - 9, 11, GOLD);
  } else if (is(w, "tree")) {
    c_.fillRoundRect(x - 5, y - 2, 10, 34, 3, PEACH);
    c_.fillCircle(x - 15, y - 8, 21, MINT); c_.fillCircle(x + 15, y - 8, 21, MINT); c_.fillCircle(x, y - 23, 23, MINT);
  } else if (is(w, "rain")) {
    c_.fillRoundRect(x - 34, y - 20, 68, 25, 12, WHITE);
    c_.fillCircle(x - 10, y - 22, 18, WHITE); c_.fillCircle(x + 14, y - 22, 14, WHITE);
    for (int i = 0; i < 4; ++i) c_.drawWideLine(x - 23 + i * 16, y + 14, x - 29 + i * 16, y + 27, 4, BLUE);
  } else if (is(w, "love")) {
    c_.fillCircle(x - 14, y - 10, 19, PINK); c_.fillCircle(x + 14, y - 10, 19, PINK);
    c_.fillTriangle(x - 31, y, x + 31, y, x, y + 32, PINK);
  } else if (is(w, "ball") || is(w, "red") || is(w, "blue") || is(w, "green") || is(w, "yellow")) {
    uint16_t color = is(w, "red") ? PINK : is(w, "blue") ? BLUE : is(w, "green") ? MINT : GOLD;
    c_.fillCircle(x, y, 30, color);
    if (is(w, "ball")) { c_.fillEllipse(x, y, 12, 30, BLUE); c_.drawFastHLine(x - 29, y, 58, WHITE); }
    c_.fillEllipse(x - 14, y - 15, 4, 7, WHITE);
  } else if (is(w, "baby") || is(w, "mom") || is(w, "dad") || is(w, "hello") || is(w, "happy")) {
    c_.fillCircle(x, y, 29, PEACH);
    if (is(w, "dad")) c_.fillRoundRect(x - 29, y - 33, 58, 13, 5, BLUE);
    if (is(w, "mom")) { c_.fillCircle(x - 24, y - 21, 14, LILAC); c_.fillCircle(x + 24, y - 21, 14, LILAC); }
    if (is(w, "baby")) c_.drawCircle(x + 2, y - 28, 6, GOLD);
    face(x, y - 3);
    if (is(w, "hello")) { c_.fillCircle(x + 44, y, 9, GOLD); c_.drawWideLine(x + 31, y + 20, x + 44, y + 3, 6, PEACH); }
  }
}

void Visuals::draw(uint32_t now) {
  float dt = previous_ ? fminf((uint32_t)(now - previous_) / 1000.f, .06f) : .033f;
  previous_ = now;
  c_.fillScreen(BG);
  c_.setTextColor(0x8c53, BG); c_.setTextDatum(top_left); c_.setTextFont(2); c_.setTextSize(1);
  c_.drawString("little wonders", 10, 5);
  c_.fillCircle(224, 12, 3, MINT);
  c_.fillEllipse(38, 151, 98, 30, 0x11e8); c_.fillEllipse(204, 160, 119, 40, 0x1209);
  for (int i = 0; i < 6; ++i) {
    int x = 20 + i * 41;
    int y = 35 + (i * 23) % 69 + sinf(now * .001f + i) * 3;
    c_.fillCircle(x, y, 1, 0x43ad);
  }
  const bool showingWord = word_ && now - wordBorn_ < 4500;
  if (showingWord) {
    float t = (now - wordBorn_) / 1000.f;
    int bob = sinf(t * 3) * 3 + 6 * expf(-t * 8);
    int sway = sinf(t * 2) * 4;
    c_.fillEllipse(120, 98, 29, 3, 0x1949);
    illustration(wordIcon_, 120 + sway, 61 + bob);
    star(54, 52, 5, MINT, t * .3f); star(189, 72, 7, LILAC, -t * .3f);
    c_.setTextDatum(top_center); c_.setTextColor(WHITE, BG); c_.setTextFont(4);
    c_.drawString(word_, 120, 104);
  } else if (!activeKey_) {
    word_ = nullptr;
    int bob = sinf(now * .002f) * 4 - 9;
    if (burstBorn_ && now - burstBorn_ < 500) bob -= sinf((now - burstBorn_) * .00628f) * 10;
    c_.fillEllipse(120, 101, 25, 3, 0x1949);
    c_.fillRoundRect(94, 52 + bob, 52, 48, 19, MINT);
    c_.fillCircle(97, 60 + bob, 10, MINT); c_.fillCircle(143, 60 + bob, 10, MINT);
    face(120, 72 + bob, 10, now % 4200 > 4050);
    c_.fillCircle(101, 81 + bob, 4, PEACH); c_.fillCircle(139, 81 + bob, 4, PEACH);
  }
  for (auto& p : particles_) {
    if (!p.alive) continue;
    float age = (uint32_t)(now - p.born) / 1000.f;
    if (age >= 2.f) { p.alive = false; continue; }
    p.x += p.vx * dt; p.y += p.vy * dt;
    if (p.kind == 0) p.vy -= 14 * dt;
    else p.vy += 65 * dt;
    if (p.y > 101 - p.r && p.vy > 0) { p.y = 101 - p.r; p.vy *= -.66f; }
    if ((p.x < p.r && p.vx < 0) || (p.x > 240 - p.r && p.vx > 0)) p.vx *= -.8f;
    float scale = fminf(1.f, age * 12.f) * fminf(1.f, (2.f - age) * 2.f);
    int r = fmaxf(1, p.r * scale);
    if (p.kind == 0) {
      c_.drawCircle(p.x, p.y, r, p.color); c_.drawCircle(p.x, p.y, r - 1, p.color);
      c_.fillCircle(p.x - r / 3, p.y - r / 3, 2, WHITE);
    } else if (p.kind == 1) {
      c_.fillCircle(p.x, p.y, r, p.color); c_.fillCircle(p.x - r / 3, p.y - r / 3, 2, WHITE);
    } else if (p.kind == 2) {
      for (int i = 0; i < 5; ++i) { float a = i * 1.256637f + age; c_.fillCircle(p.x + cosf(a) * r * .6f, p.y + sinf(a) * r * .6f, r / 2 + 1, p.color); }
      c_.fillCircle(p.x, p.y, r / 3 + 1, GOLD);
    } else star(p.x, p.y, r, p.color, age);
  }
  // Draw the latest key above particles so its letter always stays readable.
  if (!showingWord) {
    if (activeKey_) drawKey(now);
    drawInput(now);
  }
}
}
