#include <Arduino.h>
#include <M5Cardputer.h>
#include <Preferences.h>
#include <cstring>
#include "audio_assets.h"
#include "input_session.h"
#include "key_feedback.h"
#include "toy_config.h"
#include "toy_visuals.h"

namespace {
using namespace toy;
M5Canvas canvas(&M5Cardputer.Display);
Visuals visuals(canvas);
InputSession input;
Preferences prefs;
struct Settings {
  uint32_t idleMs = kDefaultIdleMs;
  uint8_t volume = kDefaultVolume;
  uint8_t brightness = kDefaultBrightness;
} settings, draft;
uint64_t previousKeys = 0;
uint32_t lastFrame = 0, lastSound = 0, parentDown = 0, lastStatus = 0;
bool parentMode = false, parentLatch = false, parentTiming = false;
bool speech = false, soundReady = false, prefsReady = false, canvasReady = false;
unsigned selection = 0;
uint32_t keyCount = 0, physicalKeyCount = 0, matchCount = 0, soundCount = 0, frames = 0;
char command[96] = {};
size_t commandLength = 0;
bool commandOverflow = false;

void logLine(const char* message) { if (Serial) Serial.println(message); }

const WordAudio* findWord(const char* word) {
  size_t lo = 0, hi = kWordCount;
  while (lo < hi) {
    size_t mid = (lo + hi) / 2;
    int compare = strcmp(word, kWords[mid].word);
    if (compare == 0) return &kWords[mid];
    if (compare < 0) hi = mid; else lo = mid + 1;
  }
  return nullptr;
}

void applySettings() {
  M5Cardputer.Speaker.setVolume(settings.volume);
  M5Cardputer.Speaker.setChannelVolume(0, 255);
  M5Cardputer.Speaker.setChannelVolume(1, kSpeechChannelVolume);
  M5Cardputer.Display.setBrightness(settings.brightness);
}

bool saveSettings() {
  if (!prefsReady) return false;
  // One blob prevents partial settings updates after a power loss.
  return prefs.putBytes("settings", &settings, sizeof(settings)) == sizeof(settings);
}

void status() {
  if (!Serial) return;
  Serial.printf("STATUS board=%d idle_ms=%lu volume=%u brightness=%u words=%u speaker=%d canvas=%d prefs=%d heap=%u keys=%lu matches=%lu sounds=%lu frames=%lu physical_keys=%lu parent=%d effects_playing=%d speech_playing=%d input=%s\n",
      int(M5.getBoard()), (unsigned long)settings.idleMs, settings.volume, settings.brightness,
      unsigned(kWordCount), soundReady, canvasReady, prefsReady, ESP.getFreeHeap(),
      (unsigned long)keyCount, (unsigned long)matchCount, (unsigned long)soundCount,
      (unsigned long)frames, (unsigned long)physicalKeyCount, parentMode,
      int(M5Cardputer.Speaker.isPlaying(0)), int(M5Cardputer.Speaker.isPlaying(1)), input.text());
}

void interruptSpeech() {
  if (speech) { M5Cardputer.Speaker.stop(1); speech = false; logLine("SPEECH interrupted"); }
  visuals.clearWord();
}

void feedback(uint16_t key, uint32_t now) {
  ++keyCount;
  interruptSpeech();
  visuals.burst(key, now);
  // Coalesce near-simultaneous key sounds, never queue a backlog.
  if (soundReady && (!soundCount || uint32_t(now - lastSound) >= kSoundSpacingMs)) {
    const auto& clip = kEffects[styleForKey(key).effect];
    bool ok = M5Cardputer.Speaker.playRaw(clip.data, clip.samples, 16000, false, 1, 0, true);
    if (ok) { ++soundCount; lastSound = now; }
    else logLine("ERROR effect playback failed");
  }
}

void finishRound(uint32_t now) {
  if (!input.due(now, settings.idleMs) || parentMode) return;
  const WordAudio* match = input.isWordCandidate() ? findWord(input.text()) : nullptr;
  if (match) {
    ++matchCount;
    visuals.showWord(match->word, match->illustration, now);
    if (soundReady) {
      M5Cardputer.Speaker.stop(0);
      speech = M5Cardputer.Speaker.playRaw(match->clip.data, match->clip.samples, 16000, false, 1, 1, true);
    }
    if (Serial) Serial.printf("ROUND match=%s speech=%d at_ms=%lu\n", match->word, speech, (unsigned long)now);
  } else logLine("ROUND no-match");
  input.clear();
  visuals.clearInput();
}

void textKey(char value, uint32_t now, uint16_t identity = 0) {
  // Expired rounds cannot be joined by the first key of a new round.
  finishRound(now);
  if (!identity) identity = value ? uint8_t(value) : kKeyFn;
  feedback(identity, now);
  if (value == '\b') input.backspace(now);
  else if (value >= 32 && value <= 126) input.append(value, now);
  else input.touch(now);
  visuals.setInput(input.text(), now, value == '\b');
  auto style = styleForKey(identity);
  if (Serial) Serial.printf("KEY at_ms=%lu length=%u identity=%u color=%u animation=%u effect=%u effect_playing=%d\n",
      (unsigned long)now, unsigned(strlen(input.text())), identity, style.color, style.animation, style.effect,
      int(M5Cardputer.Speaker.isPlaying(0)));
}

void openSettings() {
  parentMode = true; draft = settings; selection = 0;
  input.clear(); visuals.clearInput(); interruptSpeech(); M5Cardputer.Speaker.stop(0);
  logLine("SETTINGS opened");
}

void closeSettings(bool save) {
  if (save) {
    Settings before = settings;
    settings = draft;
    if (!saveSettings()) {
      settings = before;
      logLine("ERROR settings could not be saved");
      return;
    }
  }
  applySettings(); parentMode = false; input.clear(); visuals.clearInput();
  logLine(save ? "SETTINGS saved" : "SETTINGS cancelled");
}

void parentKey(char key) {
  if (key == 'w' || key == ';') selection = (selection + 2) % 3;
  if (key == 's' || key == '.') selection = (selection + 1) % 3;
  int direction = (key == 'a' || key == ',') ? -1 : (key == 'd' || key == '/') ? 1 : 0;
  if (direction) {
    if (selection == 0) draft.idleMs = constrain(int(draft.idleMs) + direction * 1000, int(kMinIdleMs), int(kMaxIdleMs));
    if (selection == 1) {
      draft.volume = constrain(int(draft.volume) + direction * 16, 0, int(kMaxVolume));
      M5Cardputer.Speaker.setVolume(draft.volume);
      const auto& clip = kEffects[7];
      if (soundReady) M5Cardputer.Speaker.playRaw(clip.data, clip.samples, 16000, false, 1, 0, true);
    }
    if (selection == 2) {
      draft.brightness = constrain(int(draft.brightness) + direction * 25, 30, 255);
      M5Cardputer.Display.setBrightness(draft.brightness);
    }
  }
  if (key == '\r') closeSettings(true);
  if (key == '`') closeSettings(false);
}

void drawSettings() {
  canvas.fillScreen(0x0926); canvas.setTextDatum(top_left);
  canvas.setTextFont(2); canvas.setTextSize(1); canvas.setTextColor(0xffbd);
  canvas.drawString("Grown-up settings", 10, 6);
  char value[36];
  for (int row = 0; row < 3; ++row) {
    int y = 29 + row * 23;
    if (row == int(selection)) canvas.fillRoundRect(6, y - 2, 228, 23, 5, 0x226d);
    if (row == 0) snprintf(value, sizeof(value), "Wait before reading     %lu s", (unsigned long)(draft.idleMs / 1000));
    if (row == 1) snprintf(value, sizeof(value), "Volume                       %u%%", unsigned(draft.volume) * 100 / 255);
    if (row == 2) snprintf(value, sizeof(value), "Brightness                   %u%%", unsigned(draft.brightness) * 100 / 255);
    canvas.drawString(value, 12, y);
  }
  canvas.setTextFont(1); canvas.setTextColor(0x9cf5);
  canvas.drawString("W/S select   A/D change", 12, 107);
  canvas.drawString("Enter save   ` cancel", 12, 121);
}

void pollKeyboard(uint32_t now) {
  uint64_t current = 0;
  for (const auto& key : M5Cardputer.Keyboard.keyList()) {
    if (key.x >= 0 && key.x < 14 && key.y >= 0 && key.y < 4)
      current |= uint64_t(1) << (key.y * 14 + key.x);
  }
  // While held (and on final release), no inactivity timeout can fire.
  if (previousKeys) input.held(now);
  uint64_t added = current & ~previousKeys;
  for (const auto& key : M5Cardputer.Keyboard.keyList()) {
    if (key.x < 0 || key.x >= 14 || key.y < 0 || key.y >= 4) continue;
    if (!(added & (uint64_t(1) << (key.y * 14 + key.x)))) continue;
    ++physicalKeyCount;
    if (Serial) Serial.printf("PHYSICAL x=%d y=%d\n", int(key.x), int(key.y));
    // Use physical first-layer keys: accidental Fn/Caps never traps toddler input.
    // English matching is case-insensitive; modifiers still get fun feedback.
    char value = M5Cardputer.Keyboard.getKeyValue(key).value_first;
    uint16_t identity = uint8_t(value);
    if (key.y == 0 && key.x == 13) value = '\b';
    else if (key.y == 2 && key.x == 13) value = '\r';
    else if (key.y == 1 && key.x == 0) value = '\t';
    if (key.y == 2 && key.x < 2) { identity = key.x == 0 ? kKeyFn : kKeyShift; value = 0; }
    else if (key.y == 3 && key.x < 3) { identity = kKeyCtrl + key.x; value = 0; }
    else identity = uint8_t(value);
    if (parentMode) parentKey(value);
    else textKey(value, now, identity);
  }
  previousKeys = current;
  if (current) input.held(now);

  if (M5Cardputer.BtnA.isPressed()) {
    if (!parentTiming) { parentTiming = true; parentDown = now; }
    if (!parentLatch && uint32_t(now - parentDown) >= kParentHoldMs) {
      parentLatch = true;
      if (parentMode) closeSettings(false); else openSettings();
    }
  } else { parentTiming = false; parentLatch = false; }
}

void sendFrame() {
  if (!canvasReady) { logLine("ERROR canvas unavailable"); return; }
  // Diagnostic export of the same RGB frame that is pushed to the LCD.
  Serial.printf("FRAME %u\n", 240u * 135u * 3u);
  uint8_t row[240 * 3];
  for (int y = 0; y < 135; ++y) {
    for (int x = 0; x < 240; ++x) {
      auto rgb = canvas.readPixelRGB(x, y);
      row[x * 3] = rgb.r; row[x * 3 + 1] = rgb.g; row[x * 3 + 2] = rgb.b;
    }
    Serial.write(row, sizeof(row));
  }
  Serial.println("\nFRAME_END");
}

bool parseNumber(const char* value, long& result) {
  char* end = nullptr;
  result = strtol(value, &end, 10);
  return value != end && *end == '\0';
}

void handleCommand(uint32_t now) {
  if (!strcmp(command, "status")) status();
  else if (!strcmp(command, "frame")) sendFrame();
  else if (!strcmp(command, "clear")) { input.clear(); visuals.clearInput(); interruptSpeech(); M5Cardputer.Speaker.stop(0); logLine("OK clear"); }
  else if (!strcmp(command, "backspace")) { if (!parentMode) textKey('\b', now); }
  else if (!strcmp(command, "settings")) openSettings();
  else if (!strcmp(command, "save")) { if (parentMode) closeSettings(true); }
  else if (!strcmp(command, "cancel")) { if (parentMode) closeSettings(false); }
  else if (!strncmp(command, "key ", 4) && strlen(command + 4) == 1) {
    if (parentMode) parentKey(command[4]); else textKey(command[4], now);
  } else if (!strncmp(command, "type ", 5)) {
    if (!parentMode) { for (const char* p = command + 5; *p; ++p) textKey(*p, now); }
  } else if (!strcmp(command, "tap")) { if (!parentMode) textKey(0, now); }
  else if (!strcmp(command, "reboot")) { logLine("OK reboot"); Serial.flush(); ESP.restart(); }
  else if (!strncmp(command, "timeout ", 8)) {
    long value;
    if (parseNumber(command + 8, value) && value >= kMinIdleMs && value <= kMaxIdleMs) {
      auto before = settings; settings.idleMs = value;
      if (saveSettings()) { input.clear(); visuals.clearInput(); logLine("OK timeout saved"); }
      else { settings = before; logLine("ERROR save failed"); }
    } else logLine("ERROR timeout must be 1000..15000 ms");
  } else logLine("ERROR command");
}

void pollSerial(uint32_t now) {
  // Bound the work per loop so a large USB write cannot starve the keyboard.
  for (unsigned count = 0; Serial.available() && count < 96; ++count) {
    char ch = Serial.read();
    if (ch == '\r') continue;
    if (ch == '\n') {
      command[commandLength] = '\0';
      if (commandOverflow) logLine("ERROR command too long");
      else if (commandLength) handleCommand(now);
      commandLength = 0; commandOverflow = false;
    } else if (commandLength < sizeof(command) - 1) command[commandLength++] = ch;
    else commandOverflow = true;
  }
}
}

void setup() {
  auto config = M5.config();
  config.internal_spk = true;
  config.internal_mic = false;
  M5Cardputer.begin(config, true);
  Serial.begin(115200);
  // Never wait for a host: the toy also starts on battery.
  M5Cardputer.Display.setRotation(1);
  canvas.setColorDepth(16);
  canvasReady = canvas.createSprite(240, 135) != nullptr;
  prefsReady = prefs.begin("little-wonders", false);
  if (prefsReady && prefs.getBytesLength("settings") == sizeof(settings))
    prefs.getBytes("settings", &settings, sizeof(settings));
  settings.idleMs = constrain(settings.idleMs, kMinIdleMs, kMaxIdleMs);
  settings.volume = min(settings.volume, kMaxVolume);
  settings.brightness = max(settings.brightness, uint8_t(30));
  applySettings();
  soundReady = M5Cardputer.Speaker.begin();
  randomSeed(esp_random());
  if (!canvasReady) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.drawString("Display memory error", 10, 40);
  }
  logLine("BOOT Little Wonders 1.3 audible play + expanded words"); status();
}

void loop() {
  M5Cardputer.update();
  uint32_t now = millis();
  pollKeyboard(now);
  pollSerial(now);
  if (!previousKeys) finishRound(now);
  if (speech && !M5Cardputer.Speaker.isPlaying(1)) { speech = false; logLine("SPEECH finished"); }
  if (canvasReady && uint32_t(now - lastFrame) >= kFrameMs) {
    lastFrame = now;
    if (parentMode) drawSettings(); else visuals.draw(now);
    canvas.pushSprite(0, 0); ++frames;
  }
  if (uint32_t(now - lastStatus) >= 30000) { lastStatus = now; status(); }
  delay(1);
}
