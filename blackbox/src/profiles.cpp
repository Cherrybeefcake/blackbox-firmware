// SPDX-License-Identifier: AGPL-3.0-or-later
#include "modules.h"
#include <Wire.h>
namespace {
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
    {"lcd",      "LCD / backlight", Bus::DisplaySPI,    6, {33, 34, 35, 36, 37, 38},   nullptr,    nullptr  },
    {"keyboard", "Keyboard bus",    Bus::InternalI2C,   3, {8, 9, 11},                 nullptr,    nullptr  },
    {"adc",      "Battery ADC",     Bus::GPIO,          1, {10},                       nullptr,    nullptr  },
    {"g0",       "G0 button",       Bus::GPIO,          1, {0},                        nullptr,    nullptr  },
    {"u031b",    "RFID2 addr 0x28", Bus::GroveI2C,      2, {2, 1},                     groveCheck, groveStop},
    {"u219-rf",  "U219 CC1101",     Bus::ExpansionSPI,  7, {40, 14, 39, 5, 13, 15, 3}, nullptr,    nullptr  },
    {"u219-nfc", "U219 ST25R3916",  Bus::ExpansionSPI,  6, {40, 14, 39, 6, 4, 3},      nullptr,    nullptr  },
    {"ext-uart", "EXT UART",        Bus::ExpansionUART, 2, {13, 15},                   nullptr,    nullptr  },
    {"sd",       "microSD",         Bus::ExpansionSPI,  4, {40, 14, 39, 12},           nullptr,    nullptr  }
};
} // namespace
unsigned moduleCount() { return sizeof(inventory) / sizeof(inventory[0]); }
Modules modules(inventory, moduleCount());
#include "platform.h"
uint16_t boardCapabilities(bool hasPartition) {
    return (modules.slot(0).owner == Owner::System ? Panel : 0) |
           (modules.slot(2).owner == Owner::System ? ADC : 0) | (hasPartition ? Flash : 0);
}
