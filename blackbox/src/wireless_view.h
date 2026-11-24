// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
void airFrame(const char *title, const char *footer);
void airText(int x, int y, const char *value, bool accent = false);
void airRect(int x, int y, int width, int height, bool accent);
void airRow(int x, int y, const char *value, bool accent);
