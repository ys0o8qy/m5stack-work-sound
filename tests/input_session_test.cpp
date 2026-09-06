#include "input_session.h"
#include "audio_assets.h"
#include "key_feedback.h"
#include <cassert>
#include <cstring>
#include <iostream>

using toy::InputSession;

int main() {
  for (uint16_t key = 'a'; key <= 'z'; ++key) {
    auto lower = toy::styleForKey(key), upper = toy::styleForKey(key - 32);
    assert(lower.color == upper.color && lower.animation == upper.animation && lower.effect == upper.effect);
    assert(lower.color < 6 && lower.animation < 4 && lower.effect < 20);
    for (uint16_t other = 'a'; other < key; ++other) {
      auto style = toy::styleForKey(other);
      assert(lower.color != style.color || lower.animation != style.animation || lower.effect != style.effect);
    }
  }
  InputSession s;
  assert(!s.due(100000, 5000));
  s.append('C', 100); s.append('a', 250); s.append('t', 400);
  assert(!s.due(5399, 5000)); assert(s.due(5400, 5000));
  assert(s.isWordCandidate() && !strcmp(s.text(), "cat"));
  s.held(6000); assert(!s.due(10999, 5000)); assert(s.due(11000, 5000));
  s.clear(); assert(!s.due(12000, 5000)); assert(!s.isWordCandidate());
  s.append('a', 100); assert(!s.due(1599, 1500)); assert(s.due(1600, 1500));
  s.append('s', 1599); assert(!s.due(3098, 1500)); assert(s.due(3099, 1500));
  s.clear(); s.touch(200); assert(!s.isWordCandidate()); assert(s.due(5200, 5000));
  s.append('c', 1); s.append('a', 2); s.append('t', 3); s.append('1', 4);
  assert(!s.isWordCandidate());
  s.backspace(5); assert(s.isWordCandidate() && !strcmp(s.text(), "cat"));
  s.append(' ', 6); assert(!s.isWordCandidate());
  s.clear(); s.backspace(1); assert(!s.isWordCandidate());
  for (unsigned i = 0; i <= toy::kMaxInput; ++i) s.append('a', i);
  assert(!s.isWordCandidate());
  for (unsigned i = 0; i < toy::kMaxInput; ++i) s.backspace(40 + i);
  s.append('c', 90); s.append('a', 91); s.append('t', 92);
  assert(!s.isWordCandidate()); // Overflow cannot later reveal a valid suffix.
  s.clear(); s.append('c', 0xffffff00u);
  assert(!s.due(0x1287u, 5000)); assert(s.due(0x1288u, 5000));
  for (size_t i = 0; i < toy::kWordCount; ++i) {
    const auto& word = toy::kWords[i];
    assert(word.clip.data && word.clip.samples >= 1600);
    assert(strlen(word.word) <= toy::kMaxInput);
    if (i) assert(strcmp(toy::kWords[i - 1].word, word.word) < 0);
    int peak = 0;
    for (size_t n = 0; n < word.clip.samples; ++n) peak = std::max(peak, std::abs(int(word.clip.data[n])));
    assert(peak >= 200 && peak <= 25000);
  }
  for (const auto& clip : toy::kEffects) {
    assert(clip.samples < 16000 / 2);
    assert(std::abs(int(clip.data[0])) < 100 && std::abs(int(clip.data[clip.samples - 1])) < 100);
  }
  std::cout << "PASS stable letter identities, timing, configurable delay, held keys, reset, normalization, invalid input, overflow, rollover, "
            << toy::kWordCount << " speech clips and 20 effects\n";
}
