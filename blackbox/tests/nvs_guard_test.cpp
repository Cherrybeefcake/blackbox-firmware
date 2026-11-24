// SPDX-License-Identifier: AGPL-3.0-or-later
#include "nvs_flash.h"
#include "workspace.h"
#include <cassert>
#include <initializer_list>
static int next;
extern "C" int __real_nvs_flash_init() { return next; }
extern "C" int __wrap_nvs_flash_init();
int main() {
    for (int code : {ESP_OK, ESP_ERR_NVS_NO_FREE_PAGES, ESP_ERR_NVS_NEW_VERSION_FOUND, 97}) {
        next = code;
        int result = __wrap_nvs_flash_init();
        assert(ws::preferencesAvailable() == (code == ESP_OK));
        assert(result == (code == 97 || code == ESP_OK ? code : ESP_FAIL));
        assert(result != ESP_ERR_NVS_NO_FREE_PAGES && result != ESP_ERR_NVS_NEW_VERSION_FOUND);
    }
}
