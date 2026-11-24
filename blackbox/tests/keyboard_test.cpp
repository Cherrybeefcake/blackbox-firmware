// SPDX-License-Identifier: AGPL-3.0-or-later
#include "Adafruit_TCA8418.h"
#include "bench.h"
#include "device.h"
#include <cassert>
TestKeyWire Wire1;
namespace keyboardFixture {
bool found;
unsigned pending, event, reads;
} // namespace keyboardFixture
namespace bench {
char keyCharacter(uint8_t, bool shift) { return shift ? 'Q' : 'q'; }
} // namespace bench
static uint32_t clockMs = 0xffffff00;
static int level = 1;
unsigned millis() { return clockMs; }
void pinMode(int, int) {}
int digitalRead(int) { return level; }
static Key button(unsigned held) {
    level = 0;
    readInput();
    clockMs += 31;
    readInput();
    clockMs += held;
    level = 1;
    readInput();
    clockMs += 31;
    return readInput();
}
int main() {
    using namespace keyboardFixture;
    assert(!initInput() && !Wire1.enabled);
    assert(button(10) == Key::Step && button(700) == Key::Hold && button(2000) == Key::Home);
    found = true;
    pending = 100;
    assert(initInput() && reads == 10 && Wire1.enabled);
    pending = 1;
    event = 0x86;
    assert(readInput() == Key::Search);
    pending = 1;
    event = 0x80 | 27;
    assert(readInput() == Key::Shortcut);
    bench::textMode(true);
    pending = 1;
    event = 0x86;
    assert(readInput() == Key::Character && bench::typed() == 'q');
    pending = 1;
    event = 0x87;
    readInput();
    pending = 1;
    event = 0x86;
    assert(readInput() == Key::Character && bench::typed() == 'Q');
    bench::textMode(false);
    assert(!bench::typed());
}
