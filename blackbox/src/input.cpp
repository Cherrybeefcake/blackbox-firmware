// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "device.h"
namespace bench {
static bool editing, shift;
static char inputChar;
void textMode(bool enable) {
    editing = enable;
    inputChar = 0;
}
char typed() { return inputChar; }
} // namespace bench
#include <Adafruit_TCA8418.h>
#include <Arduino.h>
#include <Wire.h>
static Adafruit_TCA8418 keys;
static bool ready;
bool initInput() {
    pinMode(0, INPUT_PULLUP);
    ready = Wire1.begin(8, 9) && keys.begin(0x34, &Wire1);
    if (ready) {
        keys.matrix(7, 8);
        keys.flush();
    }
    return ready;
}
Key readInput() {
    static bool sampled, stable;
    static uint32_t edge, start;
    bool down = digitalRead(0) == LOW;
    if (sampled != down) {
        sampled = down;
        edge = millis();
    }
    if (stable != sampled && millis() - edge >= 30) {
        stable = sampled;
        if (stable) start = millis();
        else return millis() - start >= 600 ? Key::Hold : Key::Step;
    }
    if (!ready || !keys.available()) return Key::None;
    // Physical row/column mapping from Bruce's Cardputer ADV matrix.
    unsigned event = keys.getEvent();
    unsigned code = event & 0x7f;
    if (code == 7) {
        bench::shift = (event & 0x80) != 0;
        return Key::None;
    }
    if (bench::editing && (event & 0x80) && code != 65 && code != 67) {
        bench::inputChar = bench::keyCharacter(code, bench::shift);
        return bench::inputChar ? Key::Character : Key::None;
    }
    switch (event) {
        case 0x80 | 6: return Key::Search;    // Q
        case 0x80 | 27: return Key::Shortcut; // F
        case 0x80 | 67: return Key::Open;
        case 0x80 | 65: return Key::Back;
        case 0x80 | 57: return Key::Up;    // ADV semicolon key
        case 0x80 | 53: return Key::Up;    // Existing alternate Up key
        case 0x80 | 54: return Key::Left;  // comma
        case 0x80 | 58: return Key::Down;  // period
        case 0x80 | 64: return Key::Right; // slash
        default: return Key::None;
    }
}
