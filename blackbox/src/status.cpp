// SPDX-License-Identifier: AGPL-3.0-or-later
#include "device.h"
#include <Arduino.h>
#include <esp_partition.h>
Status status{};
void initStatus(bool keyboard) {
    status.adv = keyboard;
    pinMode(10, INPUT);
    auto part =
        esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs");
    status.partition = part ? part->size : 0;
    sampleStatus();
}
void sampleStatus() {
    unsigned mv = analogReadMilliVolts(10) * 2;
    status.mv = mv >= 2500 && mv <= 4500 ? mv : 0;
    status.heap = ESP.getFreeHeap();
}
