// SPDX-License-Identifier: AGPL-3.0-or-later
// Exercise hardware-side session paths with test callbacks and an isolated registry.
#include "bench_stubs.h"
#include <cassert>
using namespace bench;
Profile profiles[14]{};
Modules modules(profiles, 14);
int main() {
    profiles[uartSlot] = {
        "rx", "UART", Bus::ExpansionUART, 2, {13, 15},
            uartStart, uartStop
    };
    profiles[i2cSlot] = {
        "i2c", "I2C", Bus::GroveI2C, 2, {2, 1},
            i2cStart, i2cStop
    };
    profiles[gpioSlot] = {
        "gpio", "Inputs", Bus::GPIO, 2, {2, 1},
            gpioStart, gpioStop
    };
    profiles[5] = {
        "cap", "U219 RF", Bus::ExpansionSPI, 2, {13, 15},
            nullptr, nullptr
    };
    Bench b;
    b.simulated = false;
    b.start(0);
    assert(!b.active && !fixture::opens);
    modules.identify(true);
    modules.reserveSystem(5);
    b.start(0);
    assert(!b.active && !fixture::opens);
    b.tool = I2c;
    fixture::failed = true;
    b.start(0);
    assert(!b.active && fixture::closes == 1);
    fixture::failed = false;
    b.start(0);
    assert(b.active && modules.slot(i2cSlot).owner == Owner::Application);
    Bench g;
    g.tool = Gpio;
    g.simulated = false;
    g.start(0);
    assert(!g.active);
    fixture::busError = true;
    b.poll(10);
    assert(!b.active && !strcmp(b.message, "Bus timeout") && modules.slot(i2cSlot).owner == Owner::None);
    g.start(100);
    assert(g.active);
    fixture::level = 3;
    g.poll(120);
    assert(g.value == 3);
    g.stop("Cancelled");
    assert(!g.active && modules.slot(gpioSlot).owner == Owner::None);
    g.start(100);
    g.poll(60100);
    assert(!g.active && modules.slot(gpioSlot).owner == Owner::None);
}
