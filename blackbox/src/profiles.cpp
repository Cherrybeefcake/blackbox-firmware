// SPDX-License-Identifier: AGPL-3.0-or-later
#include "modules.h"
#include "lab.h"
#include "bench.h"
#include "subghz.h"
#include <Wire.h>
namespace {
// Physical revision and electrical validation pending for all three lab profiles.
constexpr bool uartValidated=false, groveInspectorValidated=false, gpioMonitorValidated=false;
constexpr bool capValidated=false, groveValidated=false, emitterValidated=false;
bool started;
State groveCheck() {
    started = Wire.begin(2, 1, 100000);
    if (!started) return State::Error;
    Wire.setTimeOut(25);
    Wire.beginTransmission(0x28);
    auto ack = Wire.endTransmission();
    return ack == 0 ? State::Available : ack == 2 ? State::Unavailable : State::Error;
}
void groveStop() {
    if (started) Wire.end();
    started = false;
}
const Profile inventory[] = {
    {"lcd",      "LCD / backlight", Bus::DisplaySPI,    6, {33, 34, 35, 36, 37, 38},          nullptr,    nullptr  },
    {"keyboard", "Keyboard bus",    Bus::InternalI2C,   3, {8, 9, 11},                        nullptr,    nullptr  },
    {"adc",      "Battery ADC",     Bus::GPIO,          1, {10},                              nullptr,    nullptr  },
    {"g0",       "G0 button",       Bus::GPIO,          1, {0},                               nullptr,    nullptr  },
    {"u031b",    "RFID2 addr 0x28", Bus::GroveI2C,      2, {2, 1},                            groveCheck, groveStop},
    {"u219-rf",
     "U219 CC1101",                 Bus::ExpansionSPI,
     9,                                                    {40, 14, 39, 5, 13, 15, 3, 6, 12},
     rf::confirmed ? rf::initChip : nullptr,
     rf::powerDown                                                                                                 },
    {"u219-nfc", "U219 ST25R3916",  Bus::ExpansionSPI,  8, {40, 14, 39, 6, 4, 3, 5, 12}, capValidated ? lab::capStart : nullptr, lab::readerStop  },
    {"ext-uart", "EXT UART",        Bus::ExpansionUART, 2, {13, 15}, uartValidated ? bench::uartStart : nullptr, bench::uartStop  },
    {"sd",       "microSD",         Bus::ExpansionSPI,  4, {40, 14, 39, 12},                  nullptr,    nullptr  },
    {"u216", "U216 Unit NFC", Bus::GroveI2C, 2, {2, 1}, groveValidated ? lab::groveStart : nullptr, lab::readerStop},
    {"adv-ir", "ADV IR emitter", Bus::GPIO, 1, {44}, emitterValidated ? lab::emitterStart : nullptr, lab::emitterStop},
    {"bench-i2c","Grove I2C scan",Bus::GroveI2C,2,{2,1},groveInspectorValidated ? bench::i2cStart : nullptr,bench::i2cStop},
    {"bench-spi","EXT SPI diagnostics",Bus::ExpansionSPI,6,{40,14,39,5,6,12},nullptr,nullptr},
    {"bench-gpio","Grove inputs",Bus::GPIO,2,{2,1},gpioMonitorValidated ? bench::gpioStart : nullptr,bench::gpioStop}
};
} // namespace
unsigned moduleCount() { return sizeof(inventory) / sizeof(inventory[0]); }
Modules modules(inventory, moduleCount());
#include "platform.h"
uint16_t boardCapabilities(bool hasPartition) {
    return Wifi | Bluetooth | (modules.slot(0).owner == Owner::System ? Panel : 0) |
           (modules.slot(2).owner == Owner::System ? ADC : 0) | (hasPartition ? Flash : 0);
}
