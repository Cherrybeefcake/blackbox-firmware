// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
struct TestKeyWire {
    bool enabled = false;
    bool begin(int sda, int scl) { return enabled = sda == 8 && scl == 9; }
    void end() { enabled = false; }
    void setTimeOut(unsigned) {}
};
extern TestKeyWire Wire1;
