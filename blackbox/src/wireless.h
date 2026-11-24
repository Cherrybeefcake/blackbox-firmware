// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stdint.h>
#include <string.h>
namespace air {
enum class Owner { None, Explorer, Channels, Meter, Bluetooth };
enum class Phase { Idle, Scanning, Done, Cancelled, Timeout, Error };
struct Record {
    uint8_t id[6], type, channel, security;
    int8_t rssi;
    char name[33], service[18];
};
struct Results {
    Record rows[32]{};
    unsigned count = 0;
    bool limited = false;
    void add(const Record &r);
    void sort(bool channel);
};
struct Samples {
    int8_t data[32]{};
    unsigned next = 0, count = 0;
    void add(int8_t rssi);
    int at(unsigned i) const { return data[(next + 32 - count + i) % 32]; }
    void stats(int &lo, int &hi, int &avg) const;
};
struct Driver {
    bool (*start)(Owner);
    bool (*stop)(Owner);
};
struct Session {
    Owner owner = Owner::None;
    Phase phase = Phase::Idle;
    uint32_t since = 0, budget = 0;
    bool fault = false;
    bool begin(Owner who, uint32_t now, uint32_t duration, const Driver &driver);
    void end(Owner who, Phase why, const Driver &driver);
    void tick(uint32_t now, const Driver &driver);
};
extern Results results;
extern Samples samples;
extern Session session;
extern const char *error;
extern unsigned minimumHeap;
bool begin(Owner who, const Record *target = nullptr);
void poll();
void cancel();
void forget();
const char *securityName(unsigned security);
} // namespace air
