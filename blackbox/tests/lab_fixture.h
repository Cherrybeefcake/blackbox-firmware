// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "lab.h"
namespace fixture {
unsigned clock, starts, stops;
State initResult = State::Available;
lab::Phase report = lab::Phase::Running;
bool finished;
State start() {
    ++starts;
    return initResult;
}
void stop() { ++stops; }
Profile inventory[] = {
    {},
    {},
    {},
    {},
    {"rfid", "RFID2", Bus::GroveI2C, 2, {2, 1}, start, stop},
    {"rf", "CC1101", Bus::ExpansionSPI, 2, {3, 5}, start, stop},
    {"cap", "U219 ST25R3916", Bus::ExpansionSPI, 8, {40, 14, 39, 6, 4, 3, 5, 12}, start, stop},
    {},
    {"sd", "SD", Bus::ExpansionSPI, 1, {12}, start, stop},
    {"grove", "U216", Bus::GroveI2C, 2, {2, 1}, start, stop},
    {"ir", "ADV IR", Bus::GPIO, 1, {44}, start, stop}
};
} // namespace fixture
Modules modules(fixture::inventory, 11);
namespace lab {
const ReaderStatus &readerStatus(Source) {
    static ReaderStatus status;
    return status;
}
Phase pollTag(Tag &) { return fixture::report; }
unsigned nowMs() { return fixture::clock; }
bool burstFinished() { return fixture::finished; }
} // namespace lab
