// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "wireless_view.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace {
using namespace bench;
Bench session;
bool detail, direct, editing;
unsigned menu, action, page, formula;
double operands[] = {3.3, 1.0};
char draft[36]{};
const char *const names[] = {
    "UART Terminal",
    "I2C Inspector",
    "SPI Diagnostics",
    "GPIO Monitor",
    "Calculators",
    "Pinout reference",
    "System Profiler",
    "Dashboard"
};
const char *const pinPages[][5] = {
    {"EXT | 14-pin header",
     "1 G3 RESET     2 5VIN",     "3 G4 INT       4 GND",
     "5 G6 BUSY      6 5VOUT",       "7 G40 SCK      8 G8 SDA"     },
    {"EXT | continued",
     "9 G14 MOSI    10 G9 SCL",   "11 G39 MISO   12 G13 UART TX",
     "13 G5 CS      14 G15 UART RX", "SD shares SPI; SD CS G12"    },
    {"Grove | HY2.0-4P",
     "Black GND   Red 5V supply", "Yellow G2   White G1",
     "Inspector: G2 SDA / G1 SCL",   "Use 3.3 V logic on GPIO"     },
    {"Onboard peripherals",
     "LCD CLK36 MOSI35 CS37",     "LCD DC34 RST33 BL38",
     "Keypad SDA8 SCL9 IRQ11",       "IR TX44   Battery ADC10   G0"}
};
const char *const equations[] = {"Ohm's law", "RC time constant", "Parallel resistance"};
const char *const units[][3] = {
    {"V",    "kOhm", "mA"  },
    {"kOhm", "uF",   "ms"  },
    {"kOhm", "kOhm", "kOhm"}
};
void line(unsigned row, const char *s, bool accent = false) { airRow(4, 24 + 14 * row, s, accent); }
void readout() {
    char buf[48];
    switch (session.tool) {
        case Uart:
            snprintf(buf, sizeof buf, "%u 8N1  RX15 TX13  %s", baudRates[baudIndex], session.message);
            line(0, buf, true);
            for (unsigned i = 0; i < 3; ++i) line(i + 1, session.console.lines[i]);
            if (editing) {
                snprintf(buf, sizeof buf, "> %s", draft);
                line(4, buf, true);
            } else {
                snprintf(buf, sizeof buf, "Received %lu bytes", (unsigned long)session.rx);
                line(4, buf);
            }
            break;
        case I2c: {
            line(0, session.message, true);
            snprintf(buf, sizeof buf, "Grove 100 kHz   %u/112   ACK %u", session.address - 8, session.found);
            line(1, buf);
            unsigned skip = page * 24, hit = 0, printed = 0;
            for (unsigned row = 0; row < 3; ++row) {
                char *end = buf;
                *end = 0;
                for (; hit < 0x78 && printed < (row + 1) * 8; ++hit) {
                    if (!(session.acknowledged[hit / 8] & (1U << (hit % 8)))) continue;
                    if (skip) {
                        --skip;
                        continue;
                    }
                    end += snprintf(end, sizeof buf - (end - buf), "%02X ", hit);
                    ++printed;
                }
                line(2 + row, buf);
            }
            break;
        }
        case Spi:
            line(0, session.message, true);
            line(1, "Bare EXT: bridge G14 to G39");
            line(2, "Remove SD and accessories first");
            line(3, "M0 100 kHz   TX 00 A5 5A FF");
            if (session.sampled) {
                snprintf(
                    buf,
                    sizeof buf,
                    "Observed RX     %02X %02X %02X %02X",
                    session.echo[0],
                    session.echo[1],
                    session.echo[2],
                    session.echo[3]
                );
                line(4, buf);
            }
            break;
        case Gpio:
            line(0, session.message, true);
            line(1, "Grove inputs | shared ground");
            for (unsigned i = 0; i < 2 && session.sampled; ++i) {
                snprintf(
                    buf,
                    sizeof buf,
                    "G%u   %s   changes %lu",
                    2 - i,
                    (session.value >> i) & 1 ? "HIGH" : "LOW",
                    (unsigned long)session.edges[i]
                );
                line(2 + i, buf);
            }
            line(4, "20 ms samples; short pulses may miss");
            break;
        case Calculators:
            line(0, equations[formula], true);
            for (unsigned i = 0; i < 2; ++i) {
                snprintf(buf, sizeof buf, "%c  %.6g %s", 'A' + i, operands[i], units[formula][i]);
                line(i + 1, buf);
            }
            snprintf(
                buf,
                sizeof buf,
                "Result   %.6g %s",
                calculate(formula, operands[0], operands[1]),
                units[formula][2]
            );
            line(3, buf, true);
            if (editing) {
                snprintf(buf, sizeof buf, "New value > %s", draft);
                line(4, buf);
            } else line(4, session.message);
            break;
        case Pinout:
            for (unsigned i = 0; i < 5; ++i) line(i, pinPages[page][i], i == 0);
            break;
        case Profiler: {
            auto m = metrics();
            snprintf(buf, sizeof buf, "CPU %u MHz   Uptime %u s", m.mhz, m.uptime);
            line(0, buf, true);
            snprintf(buf, sizeof buf, "Free heap       %u B", m.freeHeap);
            line(1, buf);
            snprintf(buf, sizeof buf, "Lowest heap     %u B", m.minHeap);
            line(2, buf);
            snprintf(buf, sizeof buf, "Largest block   %u B", m.largest);
            line(3, buf);
            snprintf(buf, sizeof buf, "Physical flash  %u KiB", m.flash / 1024);
            line(4, buf);
            break;
        }
    }
}
const char *option(unsigned i) {
    if (editing) return session.tool == Uart ? "Enter sends CRLF | G0 cancels" : "Enter applies | G0 cancels";
    switch (session.tool) {
        case Uart:
            if (!i) return session.active ? "Close port" : "Open port";
            if (i == 1) return session.active ? "Compose line" : "Change baud";
            break;
        case I2c:
            if (!i) return session.active ? "Stop scan" : "Scan addresses";
            if (i == 1) return "Next results";
            break;
        case Gpio:
            if (!i) return session.active ? "Stop sampling" : "Start sampling";
            break;
        case Spi:
            if (!i) return "Run loopback";
            break;
        case Calculators: {
            static const char *const choices[] = {
                "Next formula", "Enter A", "Enter B", "Cycle A preset", "Cycle B preset", "Back"
            };
            return choices[i];
        }
        case Pinout:
            if (!i) return "Next pin group";
            break;
        case Profiler: break;
    }
    return "Back";
}
unsigned controls() {
    constexpr unsigned counts[] = {3, 3, 2, 2, 6, 2, 1};
    return counts[session.tool];
}
void draw() {
    airFrame(detail ? names[session.tool] : "Hardware Lab", "Arrows/G0 next  Enter/hold select");
    if (!detail) {
        unsigned first = menu < 5 ? 0 : 3;
        for (unsigned i = first; i < first + 5; ++i) {
            char buf[32];
            snprintf(buf, sizeof buf, "%c %s", i == menu ? '>' : ' ', names[i]);
            line(i - first, buf, i == menu);
        }
    } else {
        readout();
        airText(4, 100, option(action), true);
    }
}
void clearEditor() {
    editing = false;
    textMode(false);
    memset(draft, 0, sizeof draft);
}
void leave() {
    clearEditor();
    session.stop("Stopped");
    session = Bench{};
}
void enter(bool uart) {
    leave();
    direct = detail = uart;
    menu = action = page = 0;
    draw();
}
void openTool() {
    session = Bench{};
    session.tool = Tool(menu);
    action = page = 0;
    detail = true;
    draw();
}
void update() {
    // Live values are refreshed once per second by the existing application lifecycle.
    if (detail && (session.active || session.tool == Profiler)) readout();
}
void input(Key key) {
    bool active = session.active;
    session.poll(now());
    if (active && !session.active) {
        clearEditor();
        draw();
    }
    if (key == Key::None) return;
    if (editing) {
        if (key == Key::Step) {
            clearEditor();
            draw();
        } else if (key == Key::Character) {
            char c = typed();
            size_t n = strlen(draft);
            if (n + 1 < sizeof draft && (session.tool == Uart || (c >= '0' && c <= '9') || c == '.')) {
                draft[n] = c;
                draft[n + 1] = 0;
                draw();
            }
        }
        return;
    }
    bool prev = key == Key::Left || key == Key::Up;
    if (!prev && key != Key::Step && key != Key::Right && key != Key::Down) return;
    unsigned n = detail ? controls() : 8;
    unsigned &item = detail ? action : menu;
    item = (item + (prev ? n - 1 : 1)) % n;
    draw();
}
void submit() {
    if (session.tool == Uart) {
        char packet[sizeof draft + 2];
        int n = snprintf(packet, sizeof packet, "%s\r\n", draft);
        session.message = transmit(packet, unsigned(n)) ? "Sent" : "TX incomplete";
    } else {
        char *end = nullptr;
        double value = strtod(draft, &end);
        if (*draft && !*end && std::isfinite(value) && value >= 1e-6 && value <= 1e6) {
            operands[action - 1] = value;
            session.message = "Updated";
        } else session.message = "Use 0.000001 to 1000000";
    }
    clearEditor();
}
bool special(Key key) {
    if (editing && session.active && now() - session.started >= 60000) {
        session.stop("60 s limit");
        clearEditor();
        draw();
        return true;
    }
    if (editing) {
        if (key == Key::Back) {
            size_t n = strlen(draft);
            if (n) draft[n - 1] = 0;
            else clearEditor();
        } else if (key == Key::Hold) clearEditor();
        else submit();
        draw();
        return true;
    }
    if (key == Key::Back || (detail && action + 1 == controls())) {
        if (!detail || direct) return false;
        leave();
        detail = false;
        draw();
        return true;
    }
    if (!detail) {
        if (menu == 7) return false;
        openTool();
        return true;
    }
    if (session.tool <= Gpio && !action) {
        if (session.active) session.stop("Stopped");
        else {
            page = 0;
            session.start(now());
        }
    } else if (session.tool == Uart) {
        if (!session.active) baudIndex = (baudIndex + 1) % 5;
        else {
            editing = true;
            textMode(true);
        }
    } else if (session.tool == I2c) {
        unsigned pages = (session.found + 23) / 24;
        page = pages ? (page + 1) % pages : 0;
    } else if (session.tool == Pinout) page = (page + 1) % 4;
    else if (session.tool == Calculators) {
        if (!action) {
            formula = (formula + 1) % 3;
            session.message = "Ready";
        } else if (action < 3) {
            editing = true;
            textMode(true);
        } else {
            constexpr double values[] = {0.1, 1, 3.3, 4.7, 10, 47, 100, 1000};
            double &v = operands[action - 3];
            unsigned i = 0;
            while (i < 8 && values[i] <= v) ++i;
            v = values[i % 8];
            session.message = "Preset";
        }
    }
    draw();
    return true;
}
} // namespace
const Lifecycle hardwareLabApp{[] { enter(false); }, update, input, leave, special};
const Lifecycle terminalApp{[] { enter(true); }, update, input, leave, special};
