// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_timer.h>
namespace bench {
namespace {
constexpr auto port = UART_NUM_1;
bool installed, uartPins, wireStarted, spiPins;
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
    return uart_set_pin(port, 13, 15, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) == ESP_OK ? State::Available
                                                                                        : State::Error;
}
void uartStop() {
    if (installed) {
        uart_wait_tx_done(port, pdMS_TO_TICKS(45)); // Drain one composed line at 9600 baud.
        uart_driver_delete(port);
        installed = false;
    }
    if (uartPins) {
        gpio_reset_pin(GPIO_NUM_13);
        gpio_reset_pin(GPIO_NUM_15);
        uartPins = false;
    }
}
int receive(uint8_t *data, unsigned capacity) { return uart_read_bytes(port, data, capacity, 0); }
bool transmit(const char *line, unsigned len) {
    // A single nonblocking FIFO write. Never retry a partially sent line.
    return uart_tx_chars(port, line, len) == int(len);
}
State i2cStart() {
    wireStarted = Wire.begin(2, 1, 100000);
    if (wireStarted) Wire.setTimeOut(10);
    return wireStarted ? State::Available : State::Error;
}
void i2cStop() {
    if (wireStarted) Wire.end();
    wireStarted = false;
}
int probe(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission(true);
}
State spiStart() {
    if (SPI.bus()) return State::Error;
    spiPins = true;
    for (int pin : {5, 6, 12}) {
        digitalWrite(pin, HIGH);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
    SPI.begin(40, 39, 14, -1);
    return SPI.bus() ? State::Available : State::Error;
}
void spiStop() {
    if (!spiPins) return;
    spiPins = false;
    SPI.end();
    for (int pin : {5, 6, 12, 40, 39, 14}) pinMode(pin, INPUT);
}
uint8_t transfer(uint8_t byte) {
    SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
    uint8_t seen = SPI.transfer(byte);
    SPI.endTransaction();
    return seen;
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
    return {
        ESP.getCpuFreqMHz(),
        ESP.getFreeHeap(),
        ESP.getMinFreeHeap(),
        ESP.getMaxAllocHeap(),
        ESP.getFlashChipSize(),
        unsigned(esp_timer_get_time() / 1000000)
    };
}
} // namespace bench
