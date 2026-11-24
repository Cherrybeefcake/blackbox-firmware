// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "modules.h"
namespace rf {
constexpr unsigned moduleIndex = 5, bins = 17, capacity = 32;
#ifdef BLACKBOX_BENCH_CONFIRMED_U219
constexpr bool confirmed = BLACKBOX_BENCH_CONFIRMED_U219 == 1;
#else
constexpr bool confirmed = false;
#endif
struct Diagnostics {
    bool identityRead = false;
    uint8_t part = 0, revision = 0;
    unsigned readyTimeouts = 0, failures = 0;
};
extern Diagnostics diagnostics;
State initChip();
void powerDown();
struct Backend {
    State (*open)();
    bool (*tune)(unsigned khz);
    bool (*read)(int &dbm);
    void (*close)();
};
extern const Backend physical, simulated;
unsigned clockMs();
bool valid(unsigned khz);
struct Sample {
    unsigned ms;
    int dbm;
};
enum class Result { Idle, Running, Stopped, Timeout, Missing, Conflict, Fault };
struct Session {
    Result result = Result::Idle;
    unsigned center = 433900, step = 50, bin = 0, seen = 0, count = 0, head = 0;
    unsigned startMs = 0, tunedMs = 0, peakKhz = 0, activityMs = 0;
    int peak = -200, current[bins]{}, hold[bins]{};
    Sample history[capacity]{};
    bool sweep = false;
    const Backend *backend = nullptr;
    unsigned frequency() const;
    void begin(const Backend &, bool scan, unsigned centerKhz, unsigned stepKhz, unsigned now);
    void tick(unsigned now);
    void stop(Result reason = Result::Stopped);
    Sample at(unsigned i) const { return history[(head + capacity - count + i) % capacity]; }
};
} // namespace rf
