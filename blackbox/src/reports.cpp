// SPDX-License-Identifier: AGPL-3.0-or-later
#include "reports.h"
#include "bench_offline.h"
#include "device.h"
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
namespace ws {
Session recorder;
const char *groupName(unsigned g) {
    const char *const names[] = {
        "Battery voltage",
        "Free heap",
        "System profiler",
        "Wi-Fi channels",
        "Sub-GHz RSSI (SIM)",
        "Module ownership",
        "GPIO levels",
        "Calculator results"
    };
    return g < Groups ? names[g] : "Unknown";
}
const char *reportName(Report r) {
    return r == Report::Session  ? "Session summary"
           : r == Report::Health ? "Device health"
                                 : "Storage and resources";
}
bool Session::start(uint16_t selected, uint32_t now) {
    if (exists || !selected || selected >= (1u << Groups)) return false;
    *this = Session{};
    mask = selected;
    started = now;
    exists = running = true;
    return true;
}
void Session::tick(uint32_t now) {
    constexpr uint32_t day = 86400000;
    if (running && now - started >= day) stop(started + day);
}
void Session::stop(uint32_t now) {
    if (running) {
        elapsed = now - started;
        running = false;
    }
}
static Group group(unsigned m) {
    if (m >= Formula0) return Calc;
    if (m >= Channel1) return Channels;
    const Group table[] = {Battery, Heap, Profiler, ModulesUsed, SubGHz, Gpio, Gpio, Profiler};
    return table[m];
}
void Session::observe(Metric m, Origin src, double value, bool error) {
    if (m >= MetricCount || !wants(group(m))) return;
    unsigned idx = m;
    if ((m == Gpio1 || m == Gpio2) && src == Origin::Simulation) idx = MetricCount + (m - Gpio1);
    else if (src != (m == SimRssi    ? Origin::Simulation
                     : m >= Formula0 ? Origin::Calculation
                                     : Origin::Device))
        return;
    auto &s = stats[idx];
    if (error || !std::isfinite(value)) {
        if (s.errors < UINT32_MAX) ++s.errors;
        return;
    }
    if (s.count == UINT32_MAX) return;
    s.low = s.count ? std::min(s.low, value) : value;
    s.high = s.count ? std::max(s.high, value) : value;
    s.mean += (value - s.mean) / ++s.count;
}
bool csvField(char *out, size_t size, const char *s) {
    size_t used = 0;
    if (!out || !s || size < 3) return false;
    out[used++] = '"';
    for (; *s; ++s) {
        size_t n = (*s == '"') ? 2 : 1;
        if (size - used < n + 2) return false;
        if (n == 2) out[used++] = '"';
        out[used++] = *s;
    }
    out[used++] = '"';
    out[used] = 0;
    return true;
}
namespace {
struct Writer {
    char *p;
    size_t left;
    bool ok;
    char meta[100]{};
    void add(const char *format, ...) {
        if (!ok) return;
        va_list args;
        va_start(args, format);
        int n = vsnprintf(p, left, format, args);
        va_end(args);
        if (n < 0 || size_t(n) >= left) {
            ok = false;
            return;
        }
        p += n;
        left -= n;
    }
    void row(bool csv, const char *name, const char *unit, const char *source, const Stat &s) {
        char low[24] = "", high[24] = "", mean[24] = "";
        if (s.count) {
            snprintf(low, sizeof low, "%.8g", s.low);
            snprintf(high, sizeof high, "%.8g", s.high);
            snprintf(mean, sizeof mean, "%.8g", s.mean);
        }
        if (csv) {
            char label[80];
            if (!csvField(label, sizeof label, name)) {
                ok = false;
                return;
            }
            add("%s%s,%s,%s,%lu,%s,%s,%s,%lu\n",
                meta,
                label,
                unit,
                source,
                (unsigned long)s.count,
                low,
                high,
                mean,
                (unsigned long)s.errors);
        } else {
            add("%s [%s] %s\n  n=%lu min=%s max=%s avg=%s errors=%lu\n",
                name,
                unit,
                source,
                (unsigned long)s.count,
                s.count ? low : "--",
                s.count ? high : "--",
                s.count ? mean : "--",
                (unsigned long)s.errors);
        }
    }
    void single(bool csv, const char *name, const char *unit, const char *src, double v, bool valid = true) {
        Stat s;
        s.count = valid;
        s.errors = !valid;
        s.low = s.high = s.mean = v;
        row(csv, name, unit, src, s);
    }
};
} // namespace
bool makeReport(Report kind, bool csv, const Snapshot &snap, const Session &session, char *out, size_t size) {
    if (!out || !size || (kind == Report::Session && !session.exists)) return false;
    Writer w{};
    w.p = out;
    w.left = size;
    w.ok = true;
    uint32_t start = kind == Report::Session ? session.started : snap.now;
    uint32_t duration = kind == Report::Session ? (session.running ? snap.now - start : session.elapsed) : 0;
    if (csv) {
        w.add(
            "report,firmware,time_basis,start_ms,duration_ms,source_mask,measurement,unit,source,count,min,"
            "max,average,errors\n"
        );
        snprintf(
            w.meta,
            sizeof w.meta,
            "%s,%s,uptime,%lu,%lu,%u,",
            reportName(kind),
            version,
            (unsigned long)start,
            (unsigned long)duration,
            kind == Report::Session ? session.mask : 0u
        );
        // Every export, including an empty session, contains its metadata.
        w.single(true, "Duration", "ms", "UPTIME", duration);
    } else
        w.add(
            "BLACKBOX OS %s | %s\nUptime start %lu ms | duration %lu ms\n",
            version,
            reportName(kind),
            (unsigned long)start,
            (unsigned long)duration
        );
    if (kind == Report::Health) {
        w.single(csv, "Battery", "mV", "DEVICE", snap.mv, snap.mv != 0);
        w.single(csv, "Free heap", "B", "DEVICE", snap.heap);
        w.single(csv, "CPU", "MHz", "DEVICE", snap.mhz);
    } else if (kind == Report::Resources) {
        w.single(csv, "Owned modules", "slots", "DEVICE", snap.active);
        w.single(csv, "Blocked profiles", "slots", "DEVICE", snap.conflicts);
        w.single(csv, "Storage mounted", "bool", snap.simulated ? "SIM" : "DEVICE", snap.mounted);
        if (snap.mounted) {
            w.single(csv, "Volume capacity", "B", snap.simulated ? "SIM" : "DEVICE", snap.capacity);
            w.single(csv, "Volume free", "B", snap.simulated ? "SIM" : "DEVICE", snap.available);
        }
    } else {
        if (!csv) {
            w.add("Selected summaries:\n");
            for (unsigned g = 0; g < Groups; ++g)
                if (session.mask & (1 << g)) w.add("  %s\n", groupName(g));
        }
        const char *const labels[] = {
            "Battery",
            "Free heap",
            "CPU",
            "Owned modules",
            "Sub-GHz RSSI",
            "GPIO 1",
            "GPIO 2",
            "Largest heap block"
        };
        const char *const units[] = {"mV", "B", "MHz", "slots", "dBm", "level", "level", "B"};
        const char *const calcUnits[] = {
            "A",
            "V",
            "ohm",
            "W",
            "ohm",
            "ohm",
            "V",
            "ohm",
            "ohm",
            "ohm",
            "ms",
            "Hz",
            "percent",
            "ms",
            "integer"
        };
        for (unsigned i = 0; i < MetricCount + 2; ++i) {
            unsigned m = i < MetricCount ? i : Gpio1 + i - MetricCount;
            if (!(session.mask & (1 << group(m)))) continue;
            const Stat &s = session.stats[i];
            if (!s.count && !s.errors && (m >= Formula0 || i >= MetricCount)) continue;
            char channel[20];
            const char *name, *unit;
            if (m >= Formula0) {
                name = bench::formula(bench::Math(m - Formula0)).name;
                unit = calcUnits[m - Formula0];
            } else if (m >= Channel1) {
                snprintf(channel, sizeof channel, "Wi-Fi channel %u", m - Channel1 + 1);
                name = channel;
                unit = "APs";
            } else {
                name = labels[m];
                unit = units[m];
            }
            w.row(
                csv,
                name,
                unit,
                i >= MetricCount || m == SimRssi ? "SIM"
                : m >= Formula0                  ? "CALCULATED"
                                                 : "DEVICE",
                s
            );
        }
        if (!csv && (session.mask & (1 << Channels)))
            w.add("AP counts: up to 32 scan results; not utilization.\n");
    }
    return w.ok;
}
} // namespace ws
