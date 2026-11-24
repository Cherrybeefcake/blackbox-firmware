// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
namespace bench {
struct Formula {
    const char *name, *label[5], *defaults[5];
    unsigned fields;
};
enum class Math {
    Current,
    Voltage,
    Resistance,
    Power,
    Series,
    Parallel,
    Divider,
    Led,
    Band4,
    Band5,
    Period,
    Frequency,
    Pwm,
    RC,
    Base,
    Logic,
    Count
};
const Formula &formula(Math f);
struct Calculator {
    Math mode = Math::Current;
    char args[5][36]{};
    unsigned field = 0;
    void select(Math f);
    void cycle();
    bool result(char (&out)[3][40], double *value = nullptr) const;
};
struct Article {
    const char *rows[5];
};
unsigned articleCount(bool pins);
const Article &article(bool pins, unsigned index);
} // namespace bench
