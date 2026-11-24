// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <initializer_list>
constexpr int HIGH = 1, LOW = 0, OUTPUT = 1, INPUT = 0;
void digitalWrite(int pin, int level);
int digitalRead(int pin);
void pinMode(int pin, int mode);
unsigned micros();
unsigned millis();
void delay(unsigned ms);
void delayMicroseconds(unsigned us);
