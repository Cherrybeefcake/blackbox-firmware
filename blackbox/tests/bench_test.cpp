// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_offline.h"
#include "bench_stubs.h"
#include "profile_stubs.h"
#include <climits>
#include <cmath>
#include <string>
using namespace bench;
std::string screen;
void airFrame(const char *title, const char *) { screen = title; }
void airRect(int, int, int, int, bool) {}
void airText(int, int, const char *text, bool) {
    screen += text;
    screen += '\n';
}
void airRow(int x, int y, const char *s, bool b) { airText(x, y, s, b); }
void step(const Lifecycle &app, unsigned n) {
    while (n--) app.input(Key::Step);
}
void choose(const Lifecycle &app) { assert(app.special(Key::Hold)); }
void model() {
    Bench b;
    b.simulated = false;
    b.start(0);
    assert(!b.active && !fixture::opens);
    modules.identify(true);
    for (unsigned i = Uart; i <= Gpio; ++i) {
        b.tool = Tool(i);
        b.start(0);
        assert(!b.active && !fixture::opens);
    }
    modules.reserveSystem(8);
    b.tool = Spi;
    b.start(0);
    assert(!strcmp(b.message, "Resource conflict") && !fixture::opens);
    b.simulated = true;
    b.tool = I2c;
    b.start(0);
    for (unsigned i = 0; i < 112; ++i) b.poll(i);
    assert(!b.active && b.responds(0x28) && b.responds(0x50) && !b.responds(0x78) && !b.responds(0x07));
    assert(b.found == 2 && b.address == 120 && !fixture::probes);
    b.tool = Spi;
    b.start(0);
    assert(!b.active && b.sampled && b.echo[1] == 0xa5 && !fixture::opens);
    b.start(0, true);
    assert(!strcmp(b.message, "Fixture mismatch"));
    b.tool = Gpio;
    b.start(UINT_MAX - 60);
    b.sample(1, UINT_MAX - 30);
    b.sample(0, 10);
    b.sample(1, 35);
    assert(b.edges[0] == 3 && b.minPulse[0] == 25 && b.maxPulse[0] == 41);
    b.edges[0] = UINT_MAX;
    b.sample(0, 70);
    assert(b.edges[0] == UINT_MAX);
    b.poll(60000);
    assert(!b.active);
    b.tool = Uart;
    b.start(100);
    uint8_t sequence[9] = {'A', 0, 'B', 13, 10, 'x', 'y', 'z', 0xff};
    b.ingest(sequence, 9, 127);
    assert(b.log.count == 2 && b.rx == 9);
    char out[40];
    formatObservation(b.log.newest(1), true, out);
    assert(strstr(out, "27 41 00 42 0D 0A 78 79 7A"));
    formatObservation(b.log.newest(1), false, out);
    assert(strstr(out, "A.B..xyz"));
    for (unsigned i = 0; i < 40; ++i) b.ingest(sequence, 1, 150 + i);
    assert(b.log.count == 32 && b.log.newest(31).ms == 58);
    b.paused = true;
    b.ingest(sequence, 9, 250);
    assert(b.skipped == 9 && b.log.newest(0).ms == 89);
    b.clear();
    assert(!b.log.count && !b.rx);
    b.stop("Stopped");
    assert(!b.active);
    assert(keyCharacter(57, false) == ';' && keyCharacter(57, true) == ':' && !keyCharacter(99, false));
}
void offline() {
    Calculator c;
    char out[3][40];
    const char *values[] = {
        "0.0033 A",
        "3.3 V",
        "330 ohm",
        "0.1 W",
        "320 ohm",
        "50 ohm",
        "3.33333333 V",
        "300 ohm minimum",
        "1000 ohm +/- 5%",
        "2370 ohm +/- 1%",
        "1 ms",
        "1000 Hz",
        "25 %",
        "470 ms (1 tau)",
        "Dec 42",
        "VOH(min) >= VIH(min)"
    };
    for (unsigned i = 0; i < unsigned(Math::Count); ++i) {
        c.select(Math(i));
        assert(c.result(out) && !strcmp(values[i], out[0]));
    }
    c.select(Math::Base);
    for (auto p : {"0b101010", "0x2A"}) {
        strcpy(c.args[0], p);
        assert(c.result(out) && !strcmp(out[2], "Bin 101010"));
    }
    for (auto p : {"-1", "4294967296", "0x", "0b2", "42z"}) {
        strcpy(c.args[0], p);
        assert(!c.result(out));
    }
    strcpy(c.args[0], "0xffffffff");
    assert(c.result(out) && strlen(out[2]) == 36);
    c.select(Math::Current);
    for (auto p : {"0", "NaN", "inf", "1e999", "-1", "abc", ""}) {
        strcpy(c.args[1], p);
        assert(!c.result(out));
    }
    c.select(Math::Pwm);
    strcpy(c.args[1], "20");
    assert(!c.result(out));
    c.select(Math::Band5);
    strcpy(c.args[0], "Black");
    assert(!c.result(out));
    c.cycle();
    assert(c.result(out));
    for (bool pins : {false, true}) {
        for (unsigned i = 0; i < articleCount(pins); ++i) assert(article(pins, i).rows[4]);
        assert(&article(pins, 0) == &article(pins, articleCount(pins)));
    }
}
void ui() {
    auto &t = terminalApp;
    t.enter();
    choose(t);
    choose(t);
    fixture::clock += 200;
    t.input(Key::None);
    assert(t.special(Key::Back));
    step(t, 2);
    choose(t);
    assert(screen.find("4C 41 42") != std::string::npos);
    t.special(Key::Back);
    step(t, 1);
    choose(t);
    choose(t);
    assert(screen.find("HW locked") != std::string::npos);
    t.leave();
    auto &app = hardwareLabApp;
    app.enter();
    step(app, 6);
    choose(app);
    assert(screen.find("182000 B") != std::string::npos);
    choose(app);
    assert(screen.find("3821808 B") != std::string::npos);
    choose(app);
    assert(screen.find("3810 mV") != std::string::npos);
    choose(app);
    app.refresh();
    assert(screen.find("2 / 32 samples") != std::string::npos);
    app.leave();
    app.enter();
    step(app, 3);
    choose(app);
    step(app, 2);
    choose(app);
    assert(fixture::edit);
    fixture::ascii = '6';
    app.input(Key::Character);
    app.special(Key::Open);
    assert(!fixture::edit && screen.find("0.006 A") != std::string::npos);
    app.leave();
}
int main() {
    model();
    offline();
    ui();
}
