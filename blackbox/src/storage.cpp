// SPDX-License-Identifier: AGPL-3.0-or-later
#include "storage.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <strings.h>
namespace ws {
Store store;
RamVolume ram;
static constexpr char stage[] = "/BLACKBOX/BBSTAGE.TMP";
const char *describe(Err e) {
    static const char *const text[] = {
        "Ready",
        "Media missing",
        "SD bench lock",
        "Resource in use",
        "Invalid path",
        "Name in use",
        "Storage full",
        "Storage I/O error",
        "Confirm first",
        "Directory limit",
        "Cancelled",
        "Recover stage in Files"
    };
    return text[unsigned(e)];
}
bool validName(const char *s) {
    if (!s || !*s || !strcasecmp(s, "BBSTAGE.TMP")) return false;
    unsigned base = 0, ext = 0;
    bool dot = false;
    for (; *s; ++s) {
        char c = *s;
        if (c == '.') {
            if (!base || dot) return false;
            dot = true;
        } else {
            if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9') && c != '-' &&
                c != '_')
                return false;
            if (dot) {
                if (++ext > 3) return false;
            } else if (++base > 8) return false;
        }
    }
    return base && (!dot || ext);
}
bool validPath(const char *s) {
    if (!s || strnlen(s, pathSize) >= pathSize || strncmp(s, root, sizeof(root) - 1)) return false;
    s += sizeof(root) - 1;
    while (*s) {
        if (*s++ != '/') return false;
        char name[13]{};
        size_t n = strcspn(s, "/");
        if (!n || n >= sizeof name) return false;
        memcpy(name, s, n);
        if (!validName(name)) return false;
        s += n;
    }
    return true;
}
bool child(const char *dir, const char *name, char (&out)[pathSize]) {
    return validPath(dir) && validName(name) &&
           snprintf(out, sizeof out, "%s/%s", dir, name) < int(sizeof out);
}
void parent(char (&path)[pathSize]) {
    if (validPath(path) && strcmp(path, root)) *strrchr(path, '/') = 0;
}
int RamVolume::find(const char *path) const {
    if (!nodes[0].entry.dir || strncmp(path, root, sizeof(root) - 1)) return -1;
    const char *part = path + sizeof(root) - 1;
    int id = 0;
    while (*part) {
        if (*part++ != '/') return -1;
        size_t len = strcspn(part, "/");
        unsigned i = 1;
        for (; i < slots; ++i)
            if (nodes[i].folder == id && len && strlen(nodes[i].entry.name) == len &&
                !strncasecmp(part, nodes[i].entry.name, len))
                break;
        if (i == slots) return -1;
        id = int(i);
        part += len;
    }
    return id;
}
int RamVolume::folder(const char *path) const {
    char dir[pathSize];
    if (strlen(path) >= sizeof dir) return -1;
    strcpy(dir, path);
    char *slash = strrchr(dir, '/');
    if (!slash) return -1;
    *slash = 0;
    return find(dir);
}
Err RamVolume::mount() {
    if (!present) return Err::Missing;
    if (!nodes[0].entry.dir) {
        nodes[0].entry.dir = true;
        const char *path = "/BLACKBOX/DEMO.CSV";
        create(path, false);
        const char sample[] = "source,unit,value\nSIM,mV,3800\nSIM,mV,3810\n";
        append(path, reinterpret_cast<const uint8_t *>(sample), sizeof(sample) - 1);
    }
    return Err::Ok;
}
bool RamVolume::stat(const char *p, Entry &out) {
    int id = find(p);
    if (id < 0) return false;
    out = nodes[id].entry;
    return true;
}
Err RamVolume::list(const char *p, Entry *out, unsigned &n) {
    int id = find(p);
    n = 0;
    if (id < 0 || !nodes[id].entry.dir) return Err::Invalid;
    for (unsigned i = 1; i < slots; ++i) {
        const auto &node = nodes[i];
        if (*node.entry.name && node.folder == id && validName(node.entry.name)) out[n++] = node.entry;
    }
    return Err::Ok;
}
bool RamVolume::create(const char *p, bool dir) {
    int f = folder(p);
    if (f < 0 || !nodes[f].entry.dir || find(p) >= 0) return false;
    for (unsigned i = 1; i < slots; ++i)
        if (!*nodes[i].entry.name) {
            auto &n = nodes[i];
            n.folder = f;
            n.entry.dir = dir;
            strcpy(n.entry.name, strrchr(p, '/') + 1);
            n.offset = used;
            return true;
        }
    return false;
}
bool RamVolume::rename(const char *p, const char *to) {
    int id = find(p), f = folder(to);
    if (id <= 0 || f < 0 || !nodes[f].entry.dir || find(to) >= 0) return false;
    for (int i = f; i; i = nodes[i].folder)
        if (i == id) return false;
    nodes[id].folder = f;
    strcpy(nodes[id].entry.name, strrchr(to, '/') + 1);
    return true;
}
bool RamVolume::erase(const char *p, bool) {
    int id = find(p);
    if (id <= 0) return false;
    for (unsigned i = 1; i < slots; ++i)
        if (*nodes[i].entry.name && nodes[i].folder == id) return false;
    auto &node = nodes[id];
    unsigned off = node.offset, n = node.entry.size;
    memmove(data + off, data + off + n, used - off - n);
    used -= n;
    for (auto &item : nodes)
        if (item.offset > off) item.offset -= std::min<unsigned>(n, item.offset - off);
    node = {};
    return true;
}
int RamVolume::append(const char *p, const uint8_t *buf, unsigned n) {
    int id = find(p);
    if (id <= 0 || nodes[id].entry.dir || n > bytes - used) return -1;
    auto &node = nodes[id];
    unsigned pos = node.entry.size ? node.offset + node.entry.size : used;
    if (!node.entry.size) node.offset = pos;
    memmove(data + pos + n, data + pos, used - pos);
    for (auto &item : nodes)
        if (&item != &node && item.offset >= pos && *item.entry.name) item.offset += n;
    memcpy(data + pos, buf, n);
    node.entry.size += n;
    used += n;
    return n;
}
int RamVolume::read(const char *p, uint32_t pos, uint8_t *out, unsigned n) {
    int id = find(p);
    if (id <= 0 || nodes[id].entry.dir || pos > nodes[id].entry.size) return -1;
    n = std::min<uint32_t>(n, nodes[id].entry.size - pos);
    memcpy(out, data + nodes[id].offset + pos, n);
    return n;
}
Err Store::mount(Volume &v, Client who) {
    if (volume || who == Client::None) return last = Err::Busy;
    last = v.mount();
    if (last != Err::Ok) return last;
    volume = &v;
    owner = who;
    Entry orphan{};
    interrupted = v.stat(stage, orphan);
    if (interrupted && orphan.dir) return fail(Err::Io);
    return last;
}
void Store::cancel() {
    if (!busy) return;
    busy = false;
    payload = nullptr;
    if (volume && !volume->online()) {
        fail(Err::Missing);
        return;
    }
    if (volume && !volume->erase(stage, false)) {
        fail(Err::Io);
        return;
    }
    last = Err::Cancelled;
}
void Store::unmount() {
    cancel();
    if (volume) volume->close();
    volume = nullptr;
    owner = Client::None;
    interrupted = false;
}
Err Store::fail(Err e) {
    unmount();
    return last = e;
}
bool Store::ready() {
    if (volume && volume->online()) return true;
    fail(Err::Missing);
    return false;
}
uint64_t Store::capacity() { return ready() ? volume->capacity() : 0; }
uint64_t Store::freeSpace() { return ready() ? volume->freeSpace() : 0; }
Err Store::list(const char *p, Entry *rows, unsigned &n, bool size) {
    n = 0;
    if (!ready()) return last;
    if (!validPath(p)) return last = Err::Invalid;
    last = volume->list(p, rows, n);
    if (last == Err::Io || last == Err::Missing) return fail(last);
    std::sort(rows, rows + n, [size](const Entry &a, const Entry &b) {
        if (a.dir != b.dir) return a.dir;
        if (size && a.size != b.size) return a.size > b.size;
        return strcasecmp(a.name, b.name) < 0;
    });
    return last;
}
Err Store::destination(const char *p) {
    if (!ready()) return last;
    if (busy) return last = Err::Busy;
    if (!validPath(p) || !strcmp(p, root)) return last = Err::Invalid;
    Entry e{};
    if (volume->stat(p, e)) return last = Err::Exists;
    char dir[pathSize];
    strcpy(dir, p);
    parent(dir);
    return last = volume->stat(dir, e) && e.dir ? Err::Ok : Err::Invalid;
}
Err Store::makeDir(const char *p) {
    if (destination(p) != Err::Ok) return last;
    return volume->create(p, true) ? last : fail(Err::Io);
}
Err Store::rename(const char *p, const char *to) {
    if (destination(to) != Err::Ok) return last;
    if (!validPath(p) || !strcmp(p, root)) return last = Err::Invalid;
    return volume->rename(p, to) ? last : fail(Err::Io);
}
Err Store::erase(const char *p, bool yes) {
    if (!yes) return last = Err::Confirm;
    if (!ready()) return last;
    if (busy) return last = Err::Busy;
    Entry e{};
    if (!validPath(p) || !strcmp(p, root) || !volume->stat(p, e)) return last = Err::Invalid;
    return volume->erase(p, e.dir) ? last = Err::Ok : fail(Err::Io);
}
int Store::read(const char *p, uint32_t pos, uint8_t *out, unsigned n) {
    if (!ready()) return -1;
    if (!validPath(p) || !out || n > chunk) {
        last = Err::Invalid;
        return -1;
    }
    int got = volume->read(p, pos, out, n);
    if (got < 0) fail(Err::Io);
    return got;
}
Err Store::start(const char *to, uint32_t size) {
    if (interrupted) return last = Err::Recovery;
    if (size > volume->freeSpace()) return fail(Err::Full);
    if (!volume->create(stage, false)) return fail(Err::Io);
    strcpy(target, to);
    done = 0;
    length = size;
    busy = true;
    return last = Err::Ok;
}
Err Store::copy(const char *p, const char *to) {
    if (destination(to) != Err::Ok) return last;
    Entry e{};
    if (!validPath(p) || !volume->stat(p, e) || e.dir) return last = Err::Invalid;
    strcpy(source, p);
    payload = nullptr;
    return start(to, e.size);
}
Err Store::write(const char *to, const uint8_t *buf, unsigned n) {
    if (destination(to) != Err::Ok) return last;
    if (!buf && n) return last = Err::Invalid;
    source[0] = 0;
    payload = buf;
    return start(to, n);
}
Err Store::tick() {
    if (!busy || !ready()) return last;
    uint8_t buf[chunk];
    unsigned n = std::min<uint32_t>(length - done, chunk);
    if (n) {
        if (*source && volume->read(source, done, buf, n) != int(n)) return fail(Err::Io);
        if (volume->append(stage, *source ? buf : payload + done, n) != int(n)) return fail(Err::Io);
        done += n;
    }
    if (done == length) {
        Entry e{};
        if (volume->stat(target, e) || !volume->stat(stage, e) || e.size != length ||
            !volume->rename(stage, target))
            return fail(Err::Io);
        busy = false;
        payload = nullptr;
    }
    return last = Err::Ok;
}
Err Store::recover(bool confirmed, char (&name)[13]) {
    if (!confirmed) return last = Err::Confirm;
    if (!ready()) return last;
    if (busy || !interrupted) return last = Err::Invalid;
    Entry e{};
    for (unsigned i = 0; i < 64; ++i) {
        char path[pathSize];
        snprintf(name, sizeof name, "PART%04u.TMP", i);
        if (!child(root, name, path)) return last = Err::Invalid;
        if (volume->stat(path, e)) continue;
        if (!volume->rename(stage, path)) return fail(Err::Io);
        interrupted = false;
        return last = Err::Ok;
    }
    return last = Err::Exists;
}
Err Store::unique(const char *dir, unsigned seq, bool csv, char (&out)[pathSize]) {
    if (!ready()) return last;
    Entry e{};
    for (unsigned i = 0; i < 64; ++i) {
        char name[13];
        snprintf(name, sizeof name, "R%07lu.%s", (unsigned long)((seq + i) % 10000000), csv ? "CSV" : "TXT");
        if (!child(dir, name, out)) return last = Err::Invalid;
        if (!volume->stat(out, e)) return last = Err::Ok;
    }
    return last = Err::Exists;
}
} // namespace ws
