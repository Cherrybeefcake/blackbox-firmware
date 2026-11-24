// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include <cstring>
namespace bench {
unsigned baudIndex = 0;
char keyCharacter(uint8_t code, bool shift) {
    // ADV TCA8418 matrix wiring, derived from the Cardputer board key map.
    const char *const plain[] = {
        "`1234567890-=",
        " qwertyuiop[]\\",
        "  asdfghjkl;'",
        "   zxcvbnm,./ "
    };
    const char *const upper[] = {"~!@#$%^&*()_+", " QWERTYUIOP{}|", "  ASDFGHJKL:\"", "   ZXCVBNM<>? "};
    unsigned u = code % 10, t = code / 10;
    if (u < 1 || u > 8 || t > 6) return 0;
    unsigned row = (u - 1) % 4, col = t * 2 + (u - 1) / 4;
    if ((row == 1 && !col) || (row == 2 && col < 2) || (row == 3 && col < 3)) return 0;
    return (shift ? upper : plain)[row][col];
}
void Console::append(uint8_t c) {
    if (c == '\n' && lastCR) {
        lastCR = false;
        return;
    }
    lastCR = c == '\r';
    bool newline = lastCR || c == '\n';
    if (newline || col == 37) {
        memmove(lines[0], lines[1], sizeof(lines[0]) * 2);
        memset(lines[2], 0, sizeof(lines[2]));
        col = 0;
    }
    if (!newline) lines[2][col++] = (c >= 32 && c <= 126) ? char(c) : '.';
}
void Bench::stop(const char *why) {
    if (active) modules.release(slots[tool], Owner::Application);
    active = false;
    message = why;
    if (tool == Uart) console = Console{};
}
void Bench::start(unsigned time) {
    if (active || tool > Gpio) return;
    Tool selected = tool;
    *this = Bench{};
    tool = selected;
    State state = modules.acquire(slots[tool], Owner::Application);
    if (state != State::Active) {
        message = stateName(state);
        return;
    }
    active = true;
    started = time;
    last = time;
    message = "Running";
    if (tool == Spi) {
        constexpr uint8_t pattern[] = {0x00, 0xa5, 0x5a, 0xff};
        for (unsigned i = 0; i < 4; ++i) echo[i] = transfer(pattern[i]);
        sampled = true;
        stop(memcmp(pattern, echo, 4) == 0 ? "Loopback matched" : "Loopback mismatch");
    } else if (tool == Gpio) {
        value = levels();
        sampled = true;
    }
}
void Bench::poll(unsigned time) {
    if (!active) return;
    if (time - started >= 60000) {
        stop("60 s limit");
        return;
    }
    if (tool == Uart) {
        uint8_t data[256];
        int count = receive(data, sizeof data);
        if (count < 0) {
            stop("UART read error");
            return;
        }
        for (int i = 0; i < count; ++i) console.append(data[i]);
        rx += count;
    } else if (tool == I2c) {
        int result = probe(address);
        if (result == 0) {
            acknowledged[address / 8] |= 1U << (address % 8);
            ++found;
        } else if (result != 2) {
            stop(result == 5 ? "I2C timed out" : "I2C bus error");
            return;
        }
        if (++address == 0x78) stop("Scan complete");
    } else if (tool == Gpio && time - last >= 20) {
        unsigned next = levels();
        for (unsigned i = 0; i < 2; ++i) edges[i] += (next ^ value) >> i & 1U;
        value = next;
        last = time;
    }
}
double calculate(unsigned formula, double a, double b) {
    // V/kOhm = mA, kOhm * uF = ms, kOhm in parallel = kOhm.
    if (formula == 0) return a / b;
    if (formula == 1) return a * b;
    return a * b / (a + b);
}
} // namespace bench
