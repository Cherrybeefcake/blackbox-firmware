// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "platform.h"
constexpr char version[] = "0.7.0";
bool initInput();
Key readInput();
void initStatus(bool keyboard);
void sampleStatus();
struct Status {
    bool adv;
    unsigned mv, heap, partition;
};
extern Status status;
void initDisplay();
void renderHome(unsigned selected, int previous);
void showNotice(const App &app, bool missing);
void refreshBar();
