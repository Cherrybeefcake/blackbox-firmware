// SPDX-License-Identifier: AGPL-3.0-or-later
#include "wireless.h"
#include <algorithm>
namespace air {
void Results::add(const Record &r) {
    for (unsigned i = 0; i < count; ++i)
        if (rows[i].type == r.type && !memcmp(rows[i].id, r.id, 6)) {
            Record merged = r;
            if (!merged.name[0]) memcpy(merged.name, rows[i].name, sizeof merged.name);
            if (!merged.service[0]) memcpy(merged.service, rows[i].service, sizeof merged.service);
            rows[i] = merged;
            return;
        }
    if (count < 32) {
        rows[count++] = r;
        return;
    }
    limited = true;
    auto weakest = std::min_element(rows, rows + count, [](const Record &a, const Record &b) {
        return a.rssi < b.rssi;
    });
    if (r.rssi > weakest->rssi) *weakest = r;
}
void Results::sort(bool channel) {
    std::sort(rows, rows + count, [channel](const Record &a, const Record &b) {
        if (channel && a.channel != b.channel) return a.channel < b.channel;
        if (a.rssi != b.rssi) return a.rssi > b.rssi;
        return memcmp(a.id, b.id, 6) < 0;
    });
}
void Samples::add(int8_t r) {
    data[next] = r;
    next = (next + 1) % 32;
    if (count < 32) ++count;
}
void Samples::stats(int &lo, int &hi, int &avg) const {
    lo = hi = avg = 0;
    if (!count) return;
    lo = hi = at(0);
    int sum = 0;
    for (unsigned i = 0; i < count; ++i) {
        int v = at(i);
        lo = std::min(lo, v);
        hi = std::max(hi, v);
        sum += v;
    }
    avg = sum / int(count);
}
bool Session::begin(Owner who, uint32_t now, uint32_t duration, const Driver &d) {
    if (fault || who == Owner::None || owner != Owner::None) return false;
    owner = who;
    since = now;
    budget = duration;
    phase = Phase::Scanning;
    if (!d.start(who)) {
        end(who, Phase::Error, d);
        return false;
    }
    return true;
}
void Session::end(Owner who, Phase why, const Driver &d) {
    if (who == Owner::None || owner != who) return;
    fault = !d.stop(who);
    phase = fault ? Phase::Error : why;
    owner = Owner::None;
}
void Session::tick(uint32_t now, const Driver &d) {
    if (owner != Owner::None && now - since >= budget) end(owner, Phase::Timeout, d);
}
} // namespace air
