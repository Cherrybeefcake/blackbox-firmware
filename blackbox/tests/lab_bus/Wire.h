// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
struct TestWire {
    bool begin(int, int, int);
    void setTimeOut(unsigned);
    void beginTransmission(uint8_t);
    void write(uint8_t);
    void write(const uint8_t *, size_t);
    int endTransmission(bool = true);
    size_t requestFrom(uint8_t, size_t);
    int read();
    void end();
};
extern TestWire Wire;
