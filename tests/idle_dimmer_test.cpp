#include "idle_dimmer.h"
#include "parent_settings.h"
#include <cassert>
#include <iostream>

int main() {
  toy::IdleDimmer dimmer;
  dimmer.touch(100);
  assert(dimmer.brightness(30099, 30000, 130) == 130);
  assert(dimmer.brightness(30100, 30000, 130) == 130);
  assert(dimmer.brightness(30850, 30000, 130) == 75);
  assert(dimmer.brightness(31600, 30000, 130) == 20);
  assert(dimmer.brightness(999999, 0, 130) == 130);
  for (uint32_t delay : {30000u, 60000u, 120000u}) {
    dimmer.touch(500);
    assert(dimmer.brightness(500 + delay - 1, delay, 130) == 130);
    assert(dimmer.brightness(500 + delay + 1500, delay, 130) == 20);
    dimmer.touch(500 + delay + 1600);
    assert(dimmer.brightness(500 + delay + 1600, delay, 130) == 130);
  }
  dimmer.touch(32000);
  assert(dimmer.brightness(32000, 30000, 130) == 130);
  for (uint32_t t = 32000; t < 100000; t += 100) {
    dimmer.touch(t); // A held physical key keeps the screen awake.
    assert(dimmer.brightness(t, 30000, 130) == 130);
  }
  dimmer.touch(0xffffff00);
  assert(dimmer.brightness(uint32_t(0xffffff00u + 31500u), 30000, 130) == 20);
  for (unsigned normal = 30; normal <= 255; ++normal) {
    dimmer.touch(0);
    unsigned previous = normal;
    for (uint32_t t = 30000; t <= 32000; t += 10) {
      auto value = dimmer.brightness(t, 30000, normal);
      assert(value <= previous && value >= 20);
      previous = value;
    }
  }
  auto upgraded = toy::migrateSettings({3000, 80, 130});
  assert(upgraded.idleMs == 3000 && upgraded.volume == 80 && upgraded.brightness == 130);
  assert(upgraded.dimMs == 60000);
  assert(toy::validDimDelay(0) && toy::validDimDelay(30000) && toy::validDimDelay(120000));
  assert(!toy::validDimDelay(1000));
  std::cout << "PASS gradual dimming, wake/hold, off, rollover, bounds and original settings migration\n";
}
