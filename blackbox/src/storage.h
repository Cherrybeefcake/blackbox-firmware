// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
namespace ws {
constexpr char root[] = "/BLACKBOX";
constexpr unsigned pathSize = 80, chunk = 128, listLimit = 32;
struct Entry {
    char name[13];
    uint32_t size;
    bool dir;
};
bool validName(const char *name);
bool validPath(const char *path);
bool child(const char *dir, const char *name, char (&out)[pathSize]);
void parent(char (&path)[pathSize]);
enum class Err { Ok, Missing, Locked, Busy, Invalid, Exists, Full, Io, Confirm, Limit, Cancelled, Recovery };
const char *describe(Err);
enum class Client { None, Files, Report };
// Backend paths are already confined by Store. Only Store may use the reserved staging file.
struct Volume {
    virtual Err mount() = 0;
    virtual void close() = 0;
    virtual bool online() = 0;
    virtual uint64_t capacity() = 0;
    virtual uint64_t freeSpace() = 0;
    virtual bool stat(const char *, Entry &) = 0;
    virtual Err list(const char *, Entry *, unsigned &count) = 0;
    virtual bool create(const char *, bool directory) = 0;
    virtual bool rename(const char *, const char *) = 0;
    virtual bool erase(const char *, bool directory) = 0;
    virtual int read(const char *, uint32_t, uint8_t *, unsigned) = 0;
    virtual int append(const char *, const uint8_t *, unsigned) = 0;
};
struct RamVolume : Volume {
    static constexpr unsigned bytes = 8192, slots = 24;
    bool present = true;
    Err mount() override;
    void close() override {}
    bool online() override { return present; }
    uint64_t capacity() override { return bytes; }
    uint64_t freeSpace() override { return bytes - used; }
    bool stat(const char *, Entry &) override;
    Err list(const char *, Entry *, unsigned &) override;
    bool create(const char *, bool) override;
    bool rename(const char *, const char *) override;
    bool erase(const char *, bool) override;
    int read(const char *, uint32_t, uint8_t *, unsigned) override;
    int append(const char *, const uint8_t *, unsigned) override;

private:
    struct Node {
        Entry entry;
        uint8_t folder;
        uint16_t offset;
    };
    Node nodes[slots]{};
    uint8_t data[bytes]{};
    unsigned used = 0;
    int find(const char *path) const;
    int folder(const char *path) const;
};
class Store {
public:
    Client owner = Client::None;
    Err last = Err::Ok;
    bool busy = false, interrupted = false;
    uint32_t done = 0, length = 0;
    Err mount(Volume &v, Client who);
    void unmount();
    Err recover(bool confirmed, char (&name)[13]);
    bool ready();
    uint64_t capacity();
    uint64_t freeSpace();
    Err list(const char *path, Entry *rows, unsigned &count, bool bySize);
    Err makeDir(const char *path);
    Err rename(const char *from, const char *to);
    Err erase(const char *path, bool confirmed);
    int read(const char *path, uint32_t pos, uint8_t *out, unsigned n);
    Err copy(const char *from, const char *to);
    // Buffer remains owned by caller until completion or cancellation.
    Err write(const char *to, const uint8_t *data, unsigned size);
    Err tick();
    void cancel();
    Err unique(const char *dir, unsigned sequence, bool csv, char (&out)[pathSize]);

private:
    Volume *volume = nullptr;
    char source[pathSize]{}, target[pathSize]{};
    const uint8_t *payload = nullptr;
    Err destination(const char *);
    Err start(const char *, uint32_t);
    Err fail(Err);
};
extern Store store;
extern RamVolume ram;
Volume &card();
} // namespace ws
