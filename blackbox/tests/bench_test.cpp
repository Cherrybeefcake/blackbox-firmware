// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_stubs.h"
#include "profile_stubs.h"
#include <climits>
#include <cmath>
using namespace bench;
static void rows(Key k, int n) {
    while (n--) hardwareLabApp.input(k);
}
void airRow(int, int, const char *, bool) {}
void airFrame(const char *, const char *) {}
void airRect(int, int, int, int, bool) {}
void airText(int, int, const char *, bool) {}
int main() {
    Bench b;
    b.start(0);
    assert(!b.active && !fixture::opens);
    modules.identify(true);
    b.tool = I2c;
    b.start(0);
    assert(b.active);
    Bench g;
    g.tool = Gpio;
    g.start(0);
    assert(!g.active && !strcmp(g.message, "Conflict"));
    for (unsigned i = 0; i < 112; ++i) b.poll(i);
    assert(
        !b.active && b.found == 2 && b.address == 120 && (b.acknowledged[5] & 1) && (b.acknowledged[10] & 1)
    );
    b.start(0);
    fixture::busError = true;
    b.poll(20);
    assert(!strcmp(b.message, "I2C timed out") && !b.active);
    b.tool = Spi;
    b.start(0);
    assert(!b.active && b.sampled && b.echo[1] == 0xa5 && fixture::transfers == 4);
    fixture::badEcho = true;
    b.start(0);
    assert(!strcmp(b.message, "Loopback mismatch"));
    modules.reserveSystem(8);
    unsigned n = fixture::opens;
    b.start(0);
    assert(fixture::opens == n && !b.sampled);
    fixture::failed = true;
    g.start(0);
    assert(!g.active && modules.slot(gpioSlot).owner == Owner::None);
    fixture::failed = false;
    g.start(UINT_MAX - 20);
    fixture::level = 3;
    g.poll(0);
    assert(g.value == 3 && g.edges[0] == 1 && g.edges[1] == 1);
    g.poll(60000);
    assert(!g.active);
    assert(keyCharacter(57, false) == ';' && keyCharacter(57, true) == ':' && !keyCharacter(99, false));
    Console console;
    for (unsigned i = 0; i < 38; ++i) console.append('a');
    console.append('\r');
    console.append('\n');
    console.append(0);
    assert(
        strlen(console.lines[0]) == 37 && !strcmp(console.lines[1], "a") && !strcmp(console.lines[2], ".")
    );
    assert(
        fabs(calculate(0, 3.3, 1) - 3.3) < 1e-8 && calculate(1, 10, 47) == 470 && calculate(2, 5, 5) == 2.5
    );
    terminalApp.enter();
    assert(!fixture::edit && modules.slot(uartSlot).owner == Owner::None);
    terminalApp.special(Key::Open);
    terminalApp.input(Key::Step);
    terminalApp.special(Key::Open);
    assert(fixture::edit);
    fixture::ascii = 'Z';
    terminalApp.input(Key::Character);
    terminalApp.special(Key::Open);
    assert(!strcmp(fixture::tx, "Z\r\n") && !fixture::edit);
    terminalApp.special(Key::Open);
    fixture::clock = 60000;
    terminalApp.special(Key::Open);
    assert(!fixture::edit && !strcmp(fixture::tx, "Z\r\n") && modules.slot(uartSlot).owner == Owner::None);
    terminalApp.leave();
    hardwareLabApp.enter();
    rows(Key::Step, 3);
    hardwareLabApp.special(Key::Hold);
    hardwareLabApp.special(Key::Hold);
    assert(modules.slot(gpioSlot).owner == Owner::Application);
    assert(hardwareLabApp.special(Key::Back) && modules.slot(gpioSlot).owner == Owner::None);
    hardwareLabApp.leave();
}
