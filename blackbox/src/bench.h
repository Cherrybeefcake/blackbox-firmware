// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "modules.h"
#include "platform.h"
namespace bench {
enum Tool : unsigned { Uart, I2c, Spi, Gpio, Calculators, Pinout, Reference, Profiler };
constexpr unsigned uartSlot = 7, i2cSlot = 11, spiSlot = 12, gpioSlot = 13;
constexpr unsigned slots[] = {uartSlot, i2cSlot, spiSlot, gpioSlot};
constexpr unsigned baudRates[] = {9600, 19200, 38400, 57600, 115200};
extern unsigned baudIndex, i2cHz, errors, toolErrors[4];
void countError(Tool tool);
State uartStart();
void uartStop();
State i2cStart();
void i2cStop();
State gpioStart();
void gpioStop();
int receive(uint8_t *data, unsigned capacity);
int probe(uint8_t address);
unsigned levels();
unsigned now();
struct Metrics {
    unsigned mhz, freeHeap, minHeap, largest, flash, uptime, staticRam, image, partition, mv;
    const char *radio;
};
Metrics metrics();
void textMode(bool);
char typed();
char keyCharacter(uint8_t matrixCode, bool shift);
template <class T, unsigned N> struct History {
    T items[N]{};
    unsigned next = 0, count = 0;
    void add(T value) {
        items[next] = value;
        next = (next + 1) % N;
        if (count < N) ++count;
    }
    T newest(unsigned age) const { return age < count ? items[(next + N - 1 - age) % N] : T{}; }
};
struct Observation {
    unsigned ms;
    uint8_t bytes[8], count;
};
void formatObservation(Observation o, bool hex, char (&out)[40]);
struct Bench {
    Tool tool = Uart;
    bool simulated = true, active = false, paused = false, sampled = false;
    const char *message = "Ready";
    unsigned started = 0, duration = 0, last = 0, clearedAt = 0, rx = 0, skipped = 0, address = 8, found = 0,
             value = 0;
    unsigned edges[2]{}, changed[2]{}, minPulse[2]{}, maxPulse[2]{};
    uint8_t ack[16]{}, echo[4]{};
    History<Observation, 32> log;
    History<uint8_t, 32> trace;
    bool responds(unsigned a) const { return a >= 8 && a < 120 && (ack[a / 8] & (1U << (a % 8))); }
    void ingest(const uint8_t *data, unsigned n, unsigned time);
    void sample(unsigned pins, unsigned time);
    void clear();
    void start(unsigned time, bool misoLow = false);
    void stop(const char *why);
    void poll(unsigned time);
};
} // namespace bench
