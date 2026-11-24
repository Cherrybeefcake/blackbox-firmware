// SPDX-License-Identifier: AGPL-3.0-or-later
#include "workspace.h"
#include <cstring>
namespace ws {
Prefs prefs;
static bool validId(unsigned id) { return id <= deskId; }
static bool validList(const uint8_t *ids) {
    bool end = false;
    unsigned mask = 0;
    for (unsigned i = 0; i < historySize; ++i) {
        unsigned id = ids[i];
        if (id == noId) {
            end = true;
            continue;
        }
        if (end || !validId(id) || (mask & (1u << id))) return false;
        mask |= 1u << id;
    }
    return true;
}
static uint32_t checksum(const uint8_t *p) {
    uint32_t h = 2166136261u;
    for (unsigned i = 0; i < packetSize - 4; ++i) h = (h ^ p[i]) * 16777619u;
    return h;
}
void encode(const Prefs &p, uint8_t (&buf)[packetSize]) {
    static_assert(sizeof(Prefs) == 21, "Change schema version when changing settings");
    buf[0] = 'B';
    buf[1] = 'X';
    buf[2] = 1;
    memcpy(buf + 3, &p, sizeof p);
    uint32_t h = checksum(buf);
    for (unsigned i = 0; i < 4; ++i) buf[packetSize - 4 + i] = h >> (8 * i);
}
bool decode(Prefs &out, const uint8_t *b, size_t n) {
    out = Prefs{};
    if (!b || n != packetSize || b[0] != 'B' || b[1] != 'X' || b[2] != 1) return false;
    uint32_t h = checksum(b);
    for (unsigned i = 0; i < 4; ++i)
        if (b[n - 4 + i] != uint8_t(h >> (8 * i))) return false;
    Prefs p;
    memcpy(&p, b + 3, sizeof p);
    if (p.brightness > 2 || p.contrast > 1 || p.theme > 1 || !validId(p.shortcut) || p.simulation > 1 || p.accessorySim > 1 ||
        p.baud >= 5 || p.fastBus > 1 || p.sizeSort > 1 || !validList(p.favorites) || !validList(p.recents))
        return false;
    out = p;
    return true;
}
bool favorite(uint8_t id) {
    if (!validId(id)) return false;
    auto &a = prefs.favorites;
    for (unsigned i = 0; i < historySize; ++i) {
        if (a[i] == id) {
            memmove(a + i, a + i + 1, historySize - i - 1);
            a[historySize - 1] = noId;
            return true;
        }
        if (a[i] == noId) {
            a[i] = id;
            return true;
        }
    }
    return false;
}
void recent(uint8_t id) {
    if (!validId(id)) return;
    unsigned i = 0;
    while (i + 1 < historySize && prefs.recents[i] != id && prefs.recents[i] != noId) ++i;
    memmove(prefs.recents + 1, prefs.recents, i);
    prefs.recents[0] = id;
}
} // namespace ws
