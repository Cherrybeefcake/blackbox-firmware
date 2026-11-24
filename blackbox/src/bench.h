// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "modules.h"
#include "platform.h"
namespace bench {
enum Tool : unsigned { Uart, I2c, Spi, Gpio, Calculators, Pinout, Profiler };
constexpr unsigned uartSlot = 7, i2cSlot = 11, spiSlot = 12, gpioSlot = 13;
constexpr unsigned slots[] = {uartSlot, i2cSlot, spiSlot, gpioSlot};
constexpr unsigned baudRates[] = {9600, 19200, 38400, 57600, 115200};
extern unsigned baudIndex;
State uartStart();
void uartStop();
State i2cStart();
void i2cStop();
State spiStart();
void spiStop();
State gpioStart();
void gpioStop();
int receive(uint8_t *data, unsigned capacity);
bool transmit(const char *line, unsigned len);
int probe(uint8_t address);
uint8_t transfer(uint8_t byte);
unsigned levels();
unsigned now();
struct Metrics {
    unsigned mhz, freeHeap, minHeap, largest, flash, uptime;
};
Metrics metrics();
void textMode(bool);
char typed();
char keyCharacter(uint8_t matrixCode, bool shift);
struct Console {
    char lines[3][38]{};
    unsigned col = 0;
    bool lastCR = false;
    void append(uint8_t c);
};
struct Bench {
    Tool tool = Uart;
    bool active = false, sampled = false;
    const char *message = "Ready";
    uint32_t started = 0, last = 0, rx = 0, edges[2]{};
    unsigned value = 0, address = 8, found = 0;
    uint8_t acknowledged[16]{}, echo[4]{};
    Console console{};
    void start(unsigned time);
    void stop(const char *why);
    void poll(unsigned time);
};
double calculate(unsigned formula, double a, double b);
} // namespace bench
