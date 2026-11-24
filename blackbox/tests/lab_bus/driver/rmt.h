// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
using gpio_num_t = int;
constexpr int GPIO_NUM_44 = 44, RMT_CHANNEL_0 = 0, RMT_CARRIER_LEVEL_HIGH = 1, RMT_IDLE_LEVEL_LOW = 0,
              ESP_OK = 0;
struct rmt_config_t {
    int pin, channel, clk_div;
    struct {
        bool carrier_en;
        unsigned carrier_freq_hz, carrier_duty_percent;
        int carrier_level, idle_level;
    } tx_config;
};
#define RMT_DEFAULT_CONFIG_TX(pin, ch)                                                                       \
    {                                                                                                        \
        pin, ch, 0, {}                                                                                       \
    }
struct rmt_item32_t {
    unsigned duration0, level0, duration1, level1;
};
int rmt_driver_install(int, int, int);
int rmt_config(const rmt_config_t *);
int rmt_write_items(int, const rmt_item32_t *, int, bool);
int rmt_wait_tx_done(int, int);
void rmt_tx_stop(int);
void rmt_driver_uninstall(int);
void gpio_reset_pin(int);
