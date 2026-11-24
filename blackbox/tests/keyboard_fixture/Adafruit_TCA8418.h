// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "Wire.h"
namespace keyboardFixture {
extern bool found;
extern unsigned pending, event, reads;
} // namespace keyboardFixture
struct Adafruit_TCA8418 {
    bool begin(unsigned, TestKeyWire *) { return keyboardFixture::found; }
    bool matrix(unsigned rows, unsigned columns) { return rows == 7 && columns == 8; }
    unsigned available() { return keyboardFixture::pending; }
    unsigned getEvent() {
        ++keyboardFixture::reads;
        if (keyboardFixture::pending) --keyboardFixture::pending;
        return keyboardFixture::event;
    }
};
