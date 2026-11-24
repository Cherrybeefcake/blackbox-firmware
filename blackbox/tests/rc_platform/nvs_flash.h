// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
using esp_err_t = int;
constexpr int ESP_OK = 0, ESP_FAIL = -1, ESP_ERR_NVS_NO_FREE_PAGES = 0x110d,
              ESP_ERR_NVS_NEW_VERSION_FOUND = 0x1110;
