// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stdint.h>
constexpr int MSBFIRST = 1, SPI_MODE1 = 1;
struct SPISettings {
    SPISettings(int, int, int) {}
};
struct TestSPI {
    void begin(int, int, int, int);
    void beginTransaction(SPISettings);
    uint8_t transfer(uint8_t);
    void endTransaction();
    void end();
};
extern TestSPI SPI;
