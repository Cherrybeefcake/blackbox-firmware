// SPDX-License-Identifier: AGPL-3.0-or-later
#include <esp_attr.h>
#include <esp_core_dump.h>

// The stock SDK can persist task stacks, including transient diagnostic data.
// Keep the legacy flash layout for restore compatibility. RC1 never writes a dump.
extern "C" void __wrap_esp_core_dump_init() {}
extern "C" void IRAM_ATTR __wrap_esp_core_dump_to_flash(panic_info_t *) {}
