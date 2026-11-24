// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "reports.h"
#include <climits>
#include <cstdio>
#include <cstring>
namespace bench {
unsigned baudIndex = 0, i2cHz = 100000, errors = 0, toolErrors[4]{};
void countError(Tool t) {
    if (t <= Gpio && toolErrors[t] != UINT_MAX) ++toolErrors[t];
    if (errors != UINT_MAX) ++errors;
}
char keyCharacter(uint8_t code, bool shift) {
    // ADV TCA8418 matrix wiring, derived from the Cardputer board key map.
    const char *const plain[] = {"`1234567890-=", " qwertyuiop[]\\", "  asdfghjkl;'", "   zxcvbnm,./ "};
    const char *const upper[] = {"~!@#$%^&*()_+", " QWERTYUIOP{}|", "  ASDFGHJKL:\"", "   ZXCVBNM<>? "};
    unsigned u = code % 10, t = code / 10;
    if (u < 1 || u > 8 || t > 6) return 0;
    unsigned row = (u - 1) % 4, col = t * 2 + (u - 1) / 4;
    if ((row == 1 && !col) || (row == 2 && col < 2) || (row == 3 && col < 3)) return 0;
    return (shift ? upper : plain)[row][col];
}
void formatObservation(Observation o, bool hex, char (&out)[40]) {
    unsigned pos = snprintf(out, sizeof out, "%5u ", o.ms);
    for (unsigned i = 0; i < o.count && i < 8; ++i) {
        auto c = o.bytes[i];
        if (hex) pos += snprintf(out + pos, sizeof out - pos, "%02X ", c);
        else out[pos++] = c >= 32 && c <= 126 ? char(c) : '.';
    }
    out[pos] = 0;
}
void Bench::ingest(const uint8_t *p, unsigned n, unsigned time) {
    rx += n;
    if (paused) {
        skipped += n;
        return;
    }
    while (n) {
        Observation o{};
        o.ms = time - started;
        o.count = n > 8 ? 8 : n;
        memcpy(o.bytes, p, o.count);
        log.add(o);
        p += o.count;
        n -= o.count;
    }
}
void Bench::sample(unsigned pins, unsigned time) {
    for (unsigned i = 0; i < 2; ++i)
        if (sampled && ((pins ^ value) & (1U << i))) {
            if (edges[i]) {
                unsigned span = time - changed[i];
                if (!minPulse[i] || span < minPulse[i]) minPulse[i] = span;
                if (span > maxPulse[i]) maxPulse[i] = span;
            }
            if (edges[i] < UINT_MAX) ++edges[i];
            changed[i] = time;
        }
    if (ws::recorder.wants(ws::Gpio)) {
        auto origin = simulated ? ws::Origin::Simulation : ws::Origin::Device;
        ws::recorder.observe(ws::Gpio2, origin, pins & 1);
        ws::recorder.observe(ws::Gpio1, origin, (pins >> 1) & 1);
    }
    sampled = true;
    value = pins;
    trace.add(uint8_t(pins));
    last = time;
}
void Bench::clear() {
    clearedAt = duration;
    log = {};
    rx = skipped = 0;
}
void Bench::stop(const char *why) {
    if (active && !simulated) modules.release(slots[tool], Owner::Application);
    active = paused = false;
    message = why;
}
void Bench::start(unsigned time, bool misoLow) {
    if (active || tool > Gpio) return;
    Tool selected = tool;
    bool demo = simulated;
    *this = Bench{};
    tool = selected;
    simulated = demo;
    started = last = time;
    if (!simulated) {
        if (modules.blocker(slots[tool]) >= 0) {
            message = "Resource conflict";
            countError(tool);
            return;
        }
        if (tool == Spi) {
            message = "Inventory only";
            return;
        }
        State result = modules.acquire(slots[tool], Owner::Application);
        if (result != State::Active) {
            message = stateName(result);
            countError(tool);
            return;
        }
    }
    active = true;
    message = "Running";
    if (tool == Spi) {
        // Fixed internal fixture; no SPI transport exists in Hardware Lab.
        const uint8_t fixture[] = {0x00, 0xa5, 0x5a, 0xff};
        for (unsigned byte = 0; byte < sizeof echo; ++byte) {
            for (int bit = 7; bit >= 0; --bit) {
                unsigned mosi = (fixture[byte] >> bit) & 1U;
                echo[byte] = (echo[byte] << 1) | (misoLow ? 0 : mosi);
            }
        }
        sampled = true;
        bool match = !memcmp(echo, fixture, sizeof echo);
        if (!match) countError(tool);
        stop(match ? "Fixture pass" : "Fixture mismatch");
    } else if (tool == Gpio) sample(simulated ? 0 : levels(), time);
}
void Bench::poll(unsigned time) {
    if (!active) return;
    duration = time - started;
    if (duration >= 60000) {
        stop("Session limit");
        return;
    }
    if (tool == Uart) {
        if (simulated) {
            if (time - last >= 200) {
                const uint8_t frame[] = {'L', 'A', 'B', 0, uint8_t(duration / 200), 13, 10};
                ingest(frame, sizeof frame, time);
                last = time;
            }
        } else {
            uint8_t data[256];
            int n = receive(data, sizeof data);
            if (n < 0) {
                countError(tool);
                stop("UART read error");
            } else ingest(data, n, time);
        }
    } else if (tool == I2c) {
        int result = simulated ? ((address == 0x28 || address == 0x50) ? 0 : 2) : probe(address);
        if (result == 0) {
            ack[address / 8] |= 1U << (address % 8);
            ++found;
        } else if (result != 2) {
            countError(tool);
            stop(result == 5 ? "Bus timeout" : "Bus error");
            return;
        }
        if (++address == 120) stop("Scan complete");
    } else if (tool == Gpio && time - last >= 20) {
        unsigned pins = simulated ? ((duration / 150) % 2) | (((duration / 260) % 2) << 1) : levels();
        sample(pins, time);
    }
}
} // namespace bench
