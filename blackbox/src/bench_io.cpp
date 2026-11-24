// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "device.h"
#include "wireless.h"
#include <Arduino.h>
#include <esp_image_format.h>
#include <esp_ota_ops.h>
extern "C" {
extern char _data_start[], _data_end[], _bss_start[], _bss_end[];
}
#include <Wire.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_timer.h>
namespace bench {
namespace {
constexpr auto port = UART_NUM_1;
bool installed, uartPins, wireStarted;
} // namespace
State uartStart() {
    if (uart_is_driver_installed(port)) return State::Error;
    if (uart_driver_install(port, 1024, 0, 0, nullptr, 0) != ESP_OK) return State::Error;
    installed = true;
    uart_config_t config{};
    config.baud_rate = baudRates[baudIndex];
    config.data_bits = UART_DATA_8_BITS;
    config.stop_bits = UART_STOP_BITS_1;
    config.parity = UART_PARITY_DISABLE;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config.source_clk = UART_SCLK_APB;
    if (uart_param_config(port, &config) != ESP_OK) return State::Error;
    uartPins = true;
    return uart_set_pin(port, UART_PIN_NO_CHANGE, 15, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) == ESP_OK
               ? State::Available
               : State::Error;
}
void uartStop() {
    if (installed) {
        uart_driver_delete(port);
        installed = false;
    }
    if (uartPins) {
        gpio_reset_pin(GPIO_NUM_15);
        uartPins = false;
    }
}
int receive(uint8_t *data, unsigned capacity) { return uart_read_bytes(port, data, capacity, 0); }
State i2cStart() {
    wireStarted = Wire.begin(2, 1, i2cHz);
    if (wireStarted) Wire.setTimeOut(10);
    return wireStarted ? State::Available : State::Error;
}
void i2cStop() {
    if (wireStarted) Wire.end();
    wireStarted = false;
}
int probe(uint8_t address) {
    if (address < 8 || address > 119) return 4;
    Wire.beginTransmission(address);
    return Wire.endTransmission(true);
}
State gpioStart() {
    pinMode(2, INPUT);
    pinMode(1, INPUT);
    return State::Available;
}
void gpioStop() {
    pinMode(2, INPUT);
    pinMode(1, INPUT);
}
unsigned levels() { return digitalRead(2) | (digitalRead(1) << 1); }
unsigned now() { return millis(); }
Metrics metrics() {
    static const auto *part = esp_ota_get_running_partition();
    static const unsigned used = []() {
        if (!part) return 0U;
        esp_image_metadata_t metadata{};
        esp_partition_pos_t location{part->address, part->size};
        return esp_image_get_metadata(&location, &metadata) == ESP_OK ? unsigned(metadata.image_len) : 0U;
    }();
    const char *owners[] = {"Idle", "Wi-Fi Explorer", "Channel Analyzer", "RSSI Meter", "BLE Explorer"};

    return {
        ESP.getCpuFreqMHz(),
        ESP.getFreeHeap(),
        ESP.getMinFreeHeap(),
        ESP.getMaxAllocHeap(),
        ESP.getFlashChipSize(),
        unsigned(esp_timer_get_time() / 1000000),
        unsigned(uintptr_t(_data_end) - uintptr_t(_data_start) + uintptr_t(_bss_end) - uintptr_t(_bss_start)),
        used,
        part ? part->size : 0U,
        status.mv,
        air::session.fault ? "Radio fault" : owners[unsigned(air::session.owner)]
    };
}
} // namespace bench
