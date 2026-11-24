// SPDX-License-Identifier: AGPL-3.0-or-later
#include <nvs_flash.h>

// Arduino 2.0.17 erases the complete NVS partition for these two return codes.
// Preserve the initialization failure while allowing startup with volatile settings.
namespace {
bool available;
}
namespace ws {
bool preferencesAvailable() { return available; }
} // namespace ws
extern "C" esp_err_t __real_nvs_flash_init();
extern "C" esp_err_t __wrap_nvs_flash_init() {
    const esp_err_t result = __real_nvs_flash_init();
    available = result == ESP_OK;
    return result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND ? ESP_FAIL : result;
}
