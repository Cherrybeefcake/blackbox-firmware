// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "bench_offline.h"
#include "device.h"
#include "wireless_view.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
namespace {
using namespace bench;
Bench session;
Calculator calc;
bool terminal, detail, editing;
unsigned menu, action, page, offset, selected, ownerIndex, simOwner;
char draft[36];
History<unsigned, 32> heaps;
Metrics metric{};
unsigned previousTick, maxGap, ticks;
const char *const terminalMenu[] = {
    "UART Monitor", "ASCII Viewer", "Hex Viewer", "Serial Config", "Session Stats", "Dashboard"
};
const char *const labMenu[] = {
    "I2C Inspector",
    "SPI Diagnostics",
    "GPIO Monitor",
    "Calculators",
    "Pinout Library",
    "Electronics Ref",
    "System Profiler",
    "Dashboard"
};
const Tool labTools[] = {I2c, Spi, Gpio, Calculators, Pinout, Reference, Profiler};
unsigned menuCount() { return terminal ? 6 : 8; }
const char *title(unsigned i) { return terminal ? terminalMenu[i] : labMenu[i]; }
void row(unsigned i, const char *value, bool accent = false) { airRow(4, 23 + 14 * i, value, accent); }
void field(unsigned i, const char *format, ...) {
    char buf[42];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof buf, format, args);
    va_end(args);
    row(i, buf);
}
enum Action {
    Start,
    Pause,
    Clear,
    Source,
    Baud,
    Older,
    Newer,
    NextPage,
    NextAddr,
    Clock,
    NextOwner,
    Fixture,
    NextFormula,
    NextField,
    Edit,
    Preset,
    Back
};
const char *caption(Action a) {
    switch (a) {
        case Start: return session.active ? "Stop" : "Start";
        case Pause: return session.paused ? "Resume capture" : "Pause capture";
        case Clear: return "Clear observations";
        case Source: return "Change source";
        case Baud: return "Next baud rate";
        case Older: return "Older observations";
        case Newer: return "Newer observations";
        case NextPage: return "Next page";
        case NextAddr: return "Next responding address";
        case Clock: return "Change bus speed";
        case NextOwner: return "Next peripheral";
        case Fixture: return "Next bus fixture";
        case NextFormula: return "Next calculator";
        case NextField: return "Next input";
        case Edit: return "Edit selected input";
        case Preset: return "Cycle value / color";
        default: return "Back";
    }
}
struct Actions {
    Action list[9];
    unsigned count;
};
Actions choices() {
    if (terminal) {
        if (menu == 0)
            return {
                {Start, Pause, Clear, Back},
                4
            };
        if (menu <= 2)
            return {
                {Older, Newer, Pause, Clear, Back},
                5
            };
        if (menu == 3)
            return {
                {Source, Baud, Back},
                3
            };
        return {
            {Clear, Back},
            2
        };
    }
    switch (session.tool) {
        case I2c:
            return {
                {Start, NextPage, NextAddr, Clock, Source, Back},
                6
            };
        case Spi:
            return {
                {Start, NextPage, NextOwner, Fixture, Source, Back},
                6
            };
        case Gpio:
            return {
                {Start, NextPage, NextOwner, Source, Back},
                5
            };
        case Calculators:
            return {
                {NextFormula, NextField, Edit, Preset, Back},
                5
            };
        case Profiler:
            return {
                {NextPage, NextOwner, Back},
                3
            };
        default:
            return {
                {NextPage, Back},
                2
            };
    }
}
const char *src() { return session.simulated ? "SIM" : "HW"; }
void statusRow() {
    char s[40];
    snprintf(s, sizeof s, "%s | %s", src(), session.paused ? "Capture paused" : session.message);
    row(0, s, true);
}
void ownerRow(unsigned y, unsigned slot) {
    const Profile *p = modules.profile(slot);
    if (!p) {
        row(y, "Unavailable");
        return;
    }
    field(y, "%s | %s", p->name, ownerName(modules.slot(slot).owner));
}
void hardwareStatus(unsigned y, unsigned slot) {
    int conflict = modules.blocker(slot);
    if (conflict >= 0) field(y, "Conflict: %s", modules.profile(conflict)->name);
    else if (modules.identity() != Board::Adv) row(y, "ADV identity unconfirmed");
    else row(y, modules.profile(slot)->start ? "Hardware profile ready" : "HW locked: bench check pending");
}
void uartView() {
    statusRow();
    if (menu == 3) {
        field(1, "%u baud  8N1  RX only", baudRates[baudIndex]);
        row(2, "EXT G15 RX / G13 TX unused");
        row(3, "3.3 V TTL; share ground");
        hardwareStatus(4, uartSlot);
    } else if (menu == 4) {
        field(1, "Received %u B  Skipped %u B", session.rx, session.skipped);
        field(2, "Elapsed %u ms", session.duration);
        field(3, "Stored %u / 32 observations", session.log.count);
        field(
            4,
            "Mean %.1f B/s",
            session.duration > session.clearedAt
                ? 1000.0 * session.rx / (session.duration - session.clearedAt)
                : 0
        );
    } else {
        field(1, "%s | ms since start | %u B", menu == 2 ? "HEX" : "ASCII", session.rx);
        unsigned depth = std::min(offset, session.log.count);
        for (unsigned i = 0; i < 3; ++i) {
            char out[40]{};
            unsigned age = depth + 2 - i;
            if (age < session.log.count) formatObservation(session.log.newest(age), menu == 2, out);
            row(i + 2, out);
        }
    }
}
void i2cView() {
    statusRow();
    if (page == 0) {
        field(1, "7-bit map  00..7F   ACK %u", session.found);
        for (unsigned y = 0; y < 8; ++y) {
            for (unsigned x = 0; x < 16; ++x) {
                unsigned addr = 16 * y + x;
                if (addr < 8 || addr >= 120) continue;
                airRect(6 + 14 * x, 53 + 5 * y, session.responds(addr) ? 10 : 2, 3, session.responds(addr));
            }
        }
    } else if (page == 1) {
        field(1, "ACK list %u | from 0x%02X", session.found, selected);
        char buf[40];
        unsigned a = selected;
        for (unsigned y = 2; y < 5; ++y) {
            unsigned pos = 0;
            buf[0] = 0;
            while (a < 120 && pos < 24) {
                if (session.responds(a)) pos += snprintf(buf + pos, sizeof buf - pos, "%02X ", a);
                ++a;
            }
            row(y, buf);
        }
    } else if (page == 2) {
        field(1, "Selected address 0x%02X", selected);
        row(2,
            session.responds(selected)    ? "Response: ACK"
            : selected >= session.address ? "Not checked"
                                          : "No ACK");
        row(3,
            session.simulated && session.responds(selected)
                ? (selected == 0x28 ? "Fixture A (synthetic)" : "Fixture B (synthetic)")
                : "Identity unknown");
        field(4, "%u of 112 addresses probed", session.address - 8);
    } else {
        field(1, "Grove %u kHz | SDA2 SCL1", i2cHz / 1000);
        row(2, "Address only | timeout 10 ms");
        row(3, "Start on authorized bench circuit");
        hardwareStatus(4, i2cSlot);
    }
}
constexpr unsigned spiSlots[] = {5, 6, 8};
const char *const spiNames[] = {"CC1101", "NFC", "SD"};
constexpr unsigned sels[] = {5, 6, 12};
void spiView() {
    statusRow();
    unsigned i = ownerIndex % 3;
    if (page == 0) {
        row(1, "EXT SCK40 MOSI14 MISO39");
        if (!session.simulated) {
            row(2, "HW inventory | no bus transactions");
            ownerRow(3, spiSlot);
            hardwareStatus(4, spiSlot);
            return;
        }
        row(2, "Fixture: 100 kHz | mode 0 MSB");
        field(
            3,
            "Fixture: %s",
            simOwner == 4 ? "MISO held low"
            : simOwner    ? spiNames[simOwner - 1]
                          : "Loopback"
        );
        if (session.sampled)
            field(
                4,
                "RX %02X %02X %02X %02X | 32 clocks",
                session.echo[0],
                session.echo[1],
                session.echo[2],
                session.echo[3]
            );
        else row(4, "No simulated response yet");
    } else if (page == 1) {
        for (unsigned n = 0; n < 3; ++n)
            field(
                n + 1,
                "SPI --> %-7s CS%u  %s",
                spiNames[n],
                sels[n],
                session.simulated ? (simOwner == n + 1 ? "LOW" : "HIGH") : "unmeasured"
            );
        row(4, session.simulated ? "Simulated chip-select levels" : "Electrical levels not sampled");
    } else {
        row(0, "DEVICE | Module Manager", true);
        ownerRow(1, spiSlots[i]);
        field(2, "G%u chip select | active low", sels[i]);
        field(3, "Registry: %s", stateName(modules.slot(spiSlots[i]).state));
        hardwareStatus(4, spiSlot);
    }
}
void gpioView() {
    statusRow();
    if (page == 0) {
        for (unsigned i = 0; i < 2; ++i) {
            if (session.sampled)
                field(
                    i + 1,
                    "G%u  %s   changes %u",
                    2 - i,
                    (session.value >> i) & 1 ? "HIGH" : "LOW",
                    session.edges[i]
                );
            else field(i + 1, "G%u  no sample", 2 - i);
        }
        row(3, "G2 / G1 input history");
        for (unsigned n = 0; n < session.trace.count; ++n) {
            unsigned v = session.trace.newest(session.trace.count - 1 - n);
            for (unsigned i = 0; i < 2; ++i) airRect(5 + n * 7, 84 + i * 7 - ((v >> i) & 1) * 4, 5, 2, true);
        }
    } else if (page == 1) {
        for (unsigned i = 0; i < 2; ++i) {
            if (session.edges[i] >= 2)
                field(i + 1, "G%u sampled %u..%u ms", 2 - i, session.minPulse[i], session.maxPulse[i]);
            else field(i + 1, "G%u: need two transitions", 2 - i);
        }
        row(3, "20 ms minimum sampling interval");
        row(4, "Narrow pulses may be missed");
    } else {
        ownerRow(1, ownerIndex % moduleCount());
        row(2, "Only G2 and G1 monitored");
        row(3, "Protected: LCD, keypad, G0, ADC");
        hardwareStatus(4, gpioSlot);
    }
}
void profileView() {
    row(0, "DEVICE | System Profiler", true);
    if (page == 0) {
        field(1, "Heap %u B | min %u B", metric.freeHeap, metric.minHeap);
        field(2, "Largest block %u B", metric.largest);
        field(3, "Static data + BSS %u B", metric.staticRam);
        field(4, "%u MHz | uptime %u s", metric.mhz, metric.uptime);
    } else if (page == 1) {
        field(1, "BLACKBOX OS %s", version);
        field(2, "App partition %u B", metric.partition);
        if (metric.image && metric.image <= metric.partition) {
            field(3, "App image %u B", metric.image);
            field(4, "Available %u B", metric.partition - metric.image);
        } else {
            row(3, "Image usage unavailable");
            row(4, "Available capacity unknown");
        }
    } else if (page == 2) {
        ownerRow(1, ownerIndex % moduleCount());
        field(2, "Radio session: %s", metric.radio);
        if (metric.mv) field(3, "Battery %u mV", metric.mv);
        else row(3, "Battery unavailable");
        field(4, "Lab errors (HW + SIM): %u", errors);
    } else if (page == 4) {
        field(1, "UART errors %u", toolErrors[Uart]);
        field(2, "I2C errors %u", toolErrors[I2c]);
        field(3, "SPI errors %u", toolErrors[Spi]);
        field(4, "GPIO errors %u", toolErrors[Gpio]);
    } else {
        field(1, "Heap trace %u / 32 samples", heaps.count);
        field(2, "UI ticks %u | max gap %u ms", ticks, maxGap);
        unsigned lo = UINT32_MAX, hi = 0;
        for (unsigned i = 0; i < heaps.count; ++i) {
            unsigned v = heaps.newest(i);
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
        field(3, "Range %u..%u B", heaps.count ? lo : 0, hi);
        for (unsigned i = 0; i < heaps.count; ++i) {
            unsigned v = heaps.newest(heaps.count - 1 - i);
            int h = 1 + (hi > lo ? uint64_t(v - lo) * 10 / (hi - lo) : 0);
            airRect(5 + 7 * i, 92 - h, 5, h, true);
        }
    }
}
void calcView() {
    row(0, formula(calc.mode).name, true);
    auto &f = formula(calc.mode);
    if (f.fields) field(1, "%s: %.20s", f.label[calc.field], editing ? draft : calc.args[calc.field]);
    else row(1, "Datasheet checks");
    char out[3][40];
    calc.result(out);
    for (unsigned i = 0; i < 3; ++i) row(2 + i, out[i]);
}
void body() {
    if (terminal) {
        uartView();
        return;
    }
    switch (session.tool) {
        case I2c: i2cView(); break;
        case Spi: spiView(); break;
        case Gpio: gpioView(); break;
        case Calculators: calcView(); break;
        case Profiler: profileView(); break;
        default: {
            auto &a = article(session.tool == Pinout, page);
            for (unsigned i = 0; i < 5; ++i) row(i, a.rows[i], i == 0);
            break;
        }
    }
}
void draw() {
    airFrame(
        detail     ? title(menu)
        : terminal ? "Terminal"
                   : "Hardware Lab",
        "G0 next / hold select    DEL back"
    );
    if (detail) {
        body();
        airText(
            4, 101, editing ? "Type value | Enter apply | G0 cancel" : caption(choices().list[action]), true
        );
    } else {
        unsigned first = menu >= 5 ? menu - 4 : 0;
        for (unsigned i = first; i < first + 5 && i < menuCount(); ++i)
            field(i - first, "%c %s", menu == i ? '>' : ' ', title(i));
    }
}
void cancelEdit() {
    editing = false;
    textMode(false);
    memset(draft, 0, sizeof draft);
}
void leave() {
    session.stop("Stopped");
    session = Bench{};
    cancelEdit();
    heaps = {};
}
void enter(bool serial) {
    leave();
    terminal = serial;
    detail = false;
    menu = action = page = selected = offset = simOwner = ownerIndex = 0;
    calc.select(Math::Current);
    draw();
}
void open() {
    detail = true;
    action = page = offset = ownerIndex = 0;
    selected = 8;
    if (!terminal) {
        session = Bench{};
        session.tool = labTools[menu];
    }
    if (session.tool == Profiler) {
        metric = metrics();
        heaps = {};
        heaps.add(metric.freeHeap);
        ticks = maxGap = 0;
        previousTick = now();
    }
    draw();
}
void refresh() {
    if (!detail) return;
    if (session.tool == Profiler) {
        metric = metrics();
        heaps.add(metric.freeHeap);
    }
    if (session.active || session.tool == Profiler) {
        // Clear only dynamic plot areas before repainting their current samples.
        if ((session.tool == Gpio && page == 0) || (session.tool == Profiler && page == 3))
            airRow(4, 80, " ", false), airRow(4, 88, " ", false);
        body();
    }
}
void input(Key k) {
    unsigned time = now();
    bool was = session.active;
    session.poll(time);
    if (was && !session.active && detail) draw();
    if (detail && session.tool == Profiler) {
        ++ticks;
        maxGap = std::max(maxGap, time - previousTick);
        previousTick = time;
    }
    if (k == Key::None) return;
    if (editing) {
        if (k == Key::Step) cancelEdit();
        else if (k == Key::Character) {
            unsigned n = strlen(draft);
            if (n < sizeof draft - 1) {
                draft[n] = typed();
                draft[n + 1] = 0;
            }
        }
        draw();
        return;
    }
    bool prev = k == Key::Up || k == Key::Left;
    if (!prev && k != Key::Right && k != Key::Down && k != Key::Step) return;
    unsigned &v = detail ? action : menu, n = detail ? choices().count : menuCount();
    v = (v + (prev ? n - 1 : 1)) % n;
    draw();
}
unsigned pages() {
    switch (session.tool) {
        case Gpio:
        case Spi: return 3;
        case I2c:
        case Profiler: return session.tool == Profiler ? 5 : 4;
        default: return articleCount(session.tool == Pinout);
    }
}
bool special(Key k) {
    if (editing) {
        if (k == Key::Back && *draft) draft[strlen(draft) - 1] = 0;
        else {
            if (k == Key::Open) strcpy(calc.args[calc.field], draft);
            cancelEdit();
        }
        draw();
        return true;
    }
    Action a = detail ? choices().list[action] : Back;
    if (k == Key::Back || (detail && a == Back)) {
        if (!detail) return false;
        if (!terminal) session.stop("Stopped");
        detail = false;
        draw();
        return true;
    }
    if (!detail) {
        if (menu + 1 == menuCount()) return false;
        open();
        return true;
    }
    switch (a) {
        case Start:
            if (session.active) session.stop("Stopped");
            else if (session.tool == Spi && session.simulated && simOwner && simOwner < 4) {
                session.sampled = false;
                session.message = "Fixture conflict";
                countError(Spi);
            } else session.start(now(), simOwner == 4);
            offset = 0;
            break;
        case Pause:
            if (session.active) session.paused = !session.paused;
            break;
        case Clear:
            session.clear();
            offset = 0;
            break;
        case Older:
            if (session.log.count) offset = std::min(offset + 3, session.log.count - 1);
            break;
        case Newer: offset = offset >= 3 ? offset - 3 : 0; break;
        case Source: {
            bool demo = session.simulated;
            Tool t = session.tool;
            session.stop("Stopped");
            session = Bench{};
            session.tool = t;
            session.simulated = !demo;
            break;
        }
        case Baud:
            if (!session.active) baudIndex = (baudIndex + 1) % 5;
            else session.message = "Stop before configuration";
            break;
        case Clock:
            if (!session.active) i2cHz = i2cHz == 100000 ? 400000 : 100000;
            else session.message = "Stop before configuration";
            break;
        case NextPage: page = (page + 1) % pages(); break;
        case NextAddr:
            page = 2;
            for (unsigned i = 0; i < 112; ++i) {
                selected = 8 + (selected - 7) % 112;
                if (session.responds(selected)) break;
            }
            break;
        case NextOwner:
            page = 2;
            ownerIndex = (ownerIndex + 1) % (session.tool == Spi ? 3 : moduleCount());
            break;
        case Fixture: simOwner = (simOwner + 1) % 5; break;
        case NextFormula: calc.select(Math((unsigned(calc.mode) + 1) % unsigned(Math::Count))); break;
        case NextField:
            if (formula(calc.mode).fields) calc.field = (calc.field + 1) % formula(calc.mode).fields;
            break;
        case Edit:
            if (formula(calc.mode).fields) {
                editing = true;
                textMode(true);
                draft[0] = 0;
            }
            break;
        case Preset: calc.cycle(); break;
        default: break;
    }
    draw();
    return true;
}
} // namespace
const Lifecycle hardwareLabApp{[] { enter(false); }, refresh, input, leave, special};
const Lifecycle terminalApp{[] { enter(true); }, refresh, input, leave, special};
