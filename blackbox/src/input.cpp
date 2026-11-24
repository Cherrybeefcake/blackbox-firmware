// SPDX-License-Identifier: AGPL-3.0-or-later
#include "device.h"
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
    switch (keys.getEvent()) {
        case 0x80 | 67: return Key::Open;
        case 0x80 | 65: return Key::Back;
        case 0x80 | 53: return Key::Up;    // semicolon
        case 0x80 | 54: return Key::Left;  // comma
        case 0x80 | 58: return Key::Down;  // period
        case 0x80 | 64: return Key::Right; // slash
        default: return Key::None;
    }
}
