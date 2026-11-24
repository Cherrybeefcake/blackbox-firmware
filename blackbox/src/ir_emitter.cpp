// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab.h"
#include <Arduino.h>
#include <driver/rmt.h>
namespace lab {
namespace {
constexpr auto channel = RMT_CHANNEL_0;
constexpr gpio_num_t output = GPIO_NUM_44;
bool installed, configured;
rmt_item32_t envelope[1]{};
} // namespace
unsigned nowMs() { return millis(); }
State emitterStart() {
    // A finite RMT item bounds optical output independently of main-loop latency.
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(output, channel);
    config.clk_div = 80;
    config.tx_config.carrier_en = true;
    config.tx_config.carrier_freq_hz = carrierHz;
    config.tx_config.carrier_duty_percent = 33;
    config.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    if (rmt_driver_install(channel, 0, 0) != ESP_OK) return State::Error;
    installed = true;
    configured = true;
    if (rmt_config(&config) != ESP_OK) return State::Error;
    envelope[0].duration0 = markUs;
    envelope[0].level0 = 1;
    envelope[0].duration1 = 1000;
    envelope[0].level1 = 0;
    return rmt_write_items(channel, envelope, 1, false) == ESP_OK ? State::Available : State::Error;
}
bool burstFinished() { return installed && rmt_wait_tx_done(channel, 0) == ESP_OK; }
void emitterStop() {
    if (installed) {
        rmt_tx_stop(channel);
        rmt_driver_uninstall(channel);
        installed = false;
    }
    if (configured) {
        gpio_reset_pin(output);
        configured = false;
    }
}
} // namespace lab
