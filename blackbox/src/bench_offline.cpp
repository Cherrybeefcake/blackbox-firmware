// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_offline.h"
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace bench {
namespace {
const Formula formulas[] = {
    {"Ohm: current",        {"Voltage V", "Resistance ohm"},            {"3.3", "1000"},       2},
    {"Ohm: voltage",        {"Current A", "Resistance ohm"},            {"0.01", "330"},       2},
    {"Ohm: resistance",     {"Voltage V", "Current A"},                 {"3.3", "0.01"},       2},
    {"Electrical power",    {"Voltage V", "Current A"},                 {"5", "0.02"},         2},
    {"Series resistance",   {"R1 ohm", "R2 ohm"},                       {"100", "220"},        2},
    {"Parallel resistance", {"R1 ohm", "R2 ohm"},                       {"100", "100"},        2},
    {"Voltage divider",     {"Vin V", "Upper R ohm", "Lower R ohm"},    {"5", "1000", "2000"}, 3},
    {"LED series resistor", {"Supply V", "LED forward V", "Target mA"}, {"5", "2", "10"},      3},
    {"4-band resistor",
     {"Digit 1", "Digit 2", "Multiplier", "Tolerance"},
     {"Brown", "Black", "Red", "Gold"},
     4                                                                                          },
    {"5-band resistor",
     {"Digit 1", "Digit 2", "Digit 3", "Multiplier", "Tolerance"},
     {"Red", "Orange", "Violet", "Brown", "Brown"},
     5                                                                                          },
    {"Frequency to period", {"Frequency Hz"},                           {"1000"},              1},
    {"Period to frequency", {"Period ms"},                              {"1"},                 1},
    {"PWM duty cycle",      {"High time us", "Period us"},              {"25", "100"},         2},
    {"RC time constant",    {"Resistance kOhm", "Capacitance uF"},      {"10", "47"},          2},
    {"Base conversion",     {"Integer: 0b / 0x / decimal"},             {"42"},                1},
    {"Logic compatibility", {},                                         {},                    0}
};
const char *const colors[] = {
    "Black", "Brown", "Red", "Orange", "Yellow", "Green", "Blue", "Violet", "Gray", "White", "Gold", "Silver"
};
const double tolerance[] = {0, 1, 2, 0, 0, 0.5, 0.25, 0.1, 0.05, 0, 5, 10};
int color(const char *name) {
    for (int i = 0; i < 12; ++i)
        if (!strcmp(name, colors[i])) return i;
    return -1;
}
bool bands(Math m) { return m == Math::Band4 || m == Math::Band5; }
bool allowed(unsigned field, unsigned count, int c) {
    if (c < 0 || c >= 12) return false;
    if (field + 1 == count) return tolerance[c] > 0;
    if (field + 2 == count) return true;
    return c < 10 && (field || c != 0);
}
bool baseValue(const char *p, uint32_t &value) {
    unsigned base = 10;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X' || p[1] == 'b' || p[1] == 'B')) {
        base = (p[1] == 'x' || p[1] == 'X') ? 16 : 2;
        p += 2;
    }
    if (!*p) return false;
    value = 0;
    for (; *p; ++p) {
        unsigned d = *p >= '0' && *p <= '9'   ? *p - '0'
                     : *p >= 'a' && *p <= 'f' ? *p - 'a' + 10
                     : *p >= 'A' && *p <= 'F' ? *p - 'A' + 10
                                              : 16;
        if (d >= base || value > (UINT32_MAX - d) / base) return false;
        value = value * base + d;
    }
    return true;
}
} // namespace
const Formula &formula(Math f) { return formulas[unsigned(f)]; }
void Calculator::select(Math f) {
    mode = f;
    field = 0;
    memset(args, 0, sizeof args);
    auto &s = formula(f);
    for (unsigned i = 0; i < s.fields; ++i) strcpy(args[i], s.defaults[i]);
}
void Calculator::cycle() {
    auto n = formula(mode).fields;
    if (!n) return;
    if (bands(mode)) {
        int c = color(args[field]);
        do { c = (c + 1) % 12; } while (!allowed(field, n, c));
        strcpy(args[field], colors[c]);
    } else {
        const char *const presets[] = {"0", "0.1", "1", "3.3", "5", "10", "100", "1000"};
        unsigned i = 0;
        while (i < 8 && strcmp(presets[i], args[field])) ++i;
        strcpy(args[field], presets[(i + 1) % 8]);
    }
}
bool Calculator::result(char (&out)[3][40], double *numeric) const {
    memset(out, 0, sizeof out);
    auto invalid = [&]() {
        strcpy(out[0], "Invalid or out of range");
        return false;
    };
    unsigned n = formula(mode).fields;
    if (mode == Math::Logic) {
        strcpy(out[0], "VOH(min) >= VIH(min)");
        strcpy(out[1], "VOL(max) <= VIL(max)");
        strcpy(out[2], "Check ratings; level-shift 5 V");
        return true;
    }
    if (mode == Math::Base) {
        uint32_t v;
        if (!baseValue(args[0], v)) return invalid();
        if (numeric) *numeric = v;
        snprintf(out[0], 40, "Dec %lu", (unsigned long)v);
        snprintf(out[1], 40, "Hex 0x%lX", (unsigned long)v);
        strcpy(out[2], "Bin ");
        unsigned pos = 4;
        for (int bit = 31; bit >= 0; --bit)
            if (pos > 4 || (v >> bit) || !bit) out[2][pos++] = '0' + ((v >> bit) & 1);
        return true;
    }
    if (bands(mode)) {
        unsigned digits = 0;
        double multiplier = 1, tol = 0;
        for (unsigned i = 0; i < n; ++i) {
            int c = color(args[i]);
            if (!allowed(i, n, c)) return invalid();
            if (i + 1 == n) tol = tolerance[c];
            else if (i + 2 == n) multiplier = c == 10 ? 0.1 : c == 11 ? 0.01 : pow(10, c);
            else digits = digits * 10 + c;
        }
        if (numeric) *numeric = digits * multiplier;
        snprintf(out[0], 40, "%.9g ohm +/- %.3g%%", digits * multiplier, tol);
        snprintf(out[1], 40, "%s %s %s", args[0], args[1], args[2]);
        snprintf(out[2], 40, "%s %s", args[3], args[4]);
        return true;
    }
    double x[3]{};
    for (unsigned i = 0; i < n; ++i) {
        char *end;
        errno = 0;
        x[i] = strtod(args[i], &end);
        if (end == args[i] || *end || errno || !std::isfinite(x[i]) || fabs(x[i]) > 1e15) return invalid();
    }
    double a = x[0], b = x[1], c = x[2], v = 0;
    const char *unit = "";
    switch (mode) {
        case Math::Current:
            if (b <= 0) return invalid();
            v = a / b;
            unit = "A";
            break;
        case Math::Voltage:
            if (b < 0) return invalid();
            v = a * b;
            unit = "V";
            break;
        case Math::Resistance:
            if (b == 0 || a / b < 0) return invalid();
            v = a / b;
            unit = "ohm";
            break;
        case Math::Power:
            v = a * b;
            unit = "W";
            break;
        case Math::Series:
            if (a < 0 || b < 0) return invalid();
            v = a + b;
            unit = "ohm";
            break;
        case Math::Parallel:
            if (a < 0 || b < 0) return invalid();
            v = a == 0 || b == 0 ? 0 : a * b / (a + b);
            unit = "ohm";
            break;
        case Math::Divider:
            if (b < 0 || c < 0 || b + c == 0) return invalid();
            v = a * (c / (b + c));
            unit = "V";
            break;
        case Math::Led:
            if (b < 0 || a <= b || c <= 0) return invalid();
            v = (a - b) * 1000 / c;
            unit = "ohm minimum";
            break;
        case Math::Period:
        case Math::Frequency:
            if (a <= 0) return invalid();
            v = 1000 / a;
            unit = mode == Math::Period ? "ms" : "Hz";
            break;
        case Math::Pwm:
            if (a < 0 || b <= 0 || a > b) return invalid();
            v = 100 * (a / b);
            unit = "%";
            break;
        case Math::RC:
            if (a <= 0 || b <= 0) return invalid();
            v = a * b;
            unit = "ms (1 tau)";
            break;
        default: return invalid();
    }
    if (!std::isfinite(v)) return invalid();
    if (numeric) *numeric = v;
    snprintf(out[0], 40, "%.9g %s", v, unit);
    return true;
}
} // namespace bench
