// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "bench.h"
#include <cstring>
namespace bench {
namespace fixture {
unsigned clock, opens, closes, level, probes, transfers;
bool failed, badEcho, busError, edit;
char ascii, tx[40]{};
const char *pending = "";
} // namespace fixture
State mockStart() {
    ++fixture::opens;
    return fixture::failed ? State::Error : State::Available;
}
State uartStart() { return mockStart(); }
void uartStop() { ++fixture::closes; }
State i2cStart() { return mockStart(); }
void i2cStop() { ++fixture::closes; }
State gpioStart() { return mockStart(); }
void gpioStop() { ++fixture::closes; }
int receive(uint8_t *p, unsigned limit) {
    unsigned n = 0;
    while (*fixture::pending && n < limit) p[n++] = *fixture::pending++;
    return n;
}
int probe(uint8_t a) {
    ++fixture::probes;
    return fixture::busError ? 5 : a == 0x28 || a == 0x50 ? 0 : 2;
}
unsigned levels() { return fixture::level; }
unsigned now() { return fixture::clock; }
Metrics metrics() {
    return {240, 182000, 159024, 110096, 8388608, 123, 74000, 1290000, 5111808, 3810, "Idle"};
}
void textMode(bool active) { fixture::edit = active; }
char typed() { return fixture::ascii; }
} // namespace bench
