// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
namespace ws {
enum Group { Battery, Heap, Profiler, Channels, SubGHz, ModulesUsed, Gpio, Calc, Groups };
const char *groupName(unsigned);
enum Metric : unsigned {
    Millivolts,
    FreeHeap,
    CpuMHz,
    ActiveModules,
    SimRssi,
    Gpio1,
    Gpio2,
    LargestHeapBlock,
    Channel1,
    Formula0 = Channel1 + 14,
    MetricCount = Formula0 + 15
};
enum class Origin { Device, Simulation, Calculation };
struct Stat {
    uint32_t count = 0, errors = 0;
    double low = 0, high = 0, mean = 0;
};
struct Session {
    bool exists = false, running = false;
    uint16_t mask = 0;
    uint32_t started = 0, elapsed = 0;
    // GPIO can have two sources; all other metrics have a single fixed origin.
    Stat stats[MetricCount + 2]{};
    bool start(uint16_t selected, uint32_t now);
    void stop(uint32_t now);
    bool wants(Group g) const { return running && (mask & (1u << g)); }
    void observe(Metric, Origin, double, bool error = false);
};
extern Session recorder;
enum class Report { Session, Health, Resources };
struct Snapshot {
    uint32_t now, mv, heap, mhz, active, conflicts;
    uint64_t capacity, available;
    bool mounted, simulated;
};
const char *reportName(Report);
bool csvField(char *out, size_t size, const char *value);
constexpr unsigned reportLimit = 6400;
bool makeReport(Report, bool csv, const Snapshot &, const Session &, char *out, size_t size);
} // namespace ws
