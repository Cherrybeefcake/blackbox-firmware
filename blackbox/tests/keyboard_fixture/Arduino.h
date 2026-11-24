// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
constexpr int INPUT_PULLUP = 2, LOW = 0;
void pinMode(int, int);
int digitalRead(int);
unsigned millis();
