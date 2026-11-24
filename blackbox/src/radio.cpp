// SPDX-License-Identifier: AGPL-3.0-or-later
#include "reports.h"
#include "wireless.h"
#include <Arduino.h>
#include <algorithm>
#include <atomic>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_event.h>
#include <esp_gap_ble_api.h>
#include <esp_netif.h>
#include <esp_wifi.h>
extern "C" bool btInUse() { return true; } // Keep controller RAM available; initialization remains explicit.
namespace air {
Results results;
Samples samples;
Session session;
const char *error = "";
unsigned minimumHeap;
namespace {
Record target;
Results advertisements;
portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
std::atomic<bool> accepting{false}, ready{false}, complete{false}, failed{false};
bool wifiInit, wifiStarted, registered, btInit, btEnabled, hostInit, hostEnabled, scanPending;
esp_event_handler_instance_t handler;
uint32_t lastScan;
unsigned attempts;
void nameCopy(char *dst, const uint8_t *src, unsigned n) {
    n = n > 32 ? 32 : n;
    for (unsigned i = 0; i < n; ++i) dst[i] = src[i] >= 32 && src[i] < 127 ? char(src[i]) : '?';
    dst[n] = 0;
}
void gap(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *p) {
    if (!accepting.load()) return;
    if (event == ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT) {
        if (p->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS) ready = true;
        else failed = true;
    } else if (event == ESP_GAP_BLE_SCAN_START_COMPLETE_EVT &&
               p->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        failed = true;
    else if (event == ESP_GAP_BLE_SCAN_RESULT_EVT) {
        auto &s = p->scan_rst;
        if (s.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT) {
            complete = true;
            return;
        }
        if (s.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) return;
        Record r{};
        memcpy(r.id, s.bda, 6);
        r.type = s.ble_addr_type;
        r.rssi = s.rssi;
        for (unsigned i = 0; i < s.adv_data_len;) {
            unsigned len = s.ble_adv[i++];
            if (!len || i + len > s.adv_data_len) break;
            uint8_t kind = s.ble_adv[i];
            const uint8_t *v = s.ble_adv + i + 1;
            if (kind == 9 || (kind == 8 && !r.name[0])) nameCopy(r.name, v, len - 1);
            if ((kind == 2 || kind == 3 || kind == 0x16) && len >= 3) {
                unsigned uuid = v[0] | (unsigned(v[1]) << 8);
                const char *cat = uuid == 0x180f   ? "Battery"
                                  : uuid == 0x180d ? "Heart rate"
                                  : uuid == 0x181a ? "Environment"
                                  : uuid == 0x1812 ? "HID"
                                                   : nullptr;
                if (cat) snprintf(r.service, sizeof r.service, "%s", cat);
                else snprintf(r.service, sizeof r.service, "UUID %04X", uuid);
            }
            if ((kind == 6 || kind == 7) && len >= 17 && !r.service[0]) strcpy(r.service, "128-bit service");
            i += len;
        }
        portENTER_CRITICAL(&lock);
        if (accepting.load()) advertisements.add(r);
        portEXIT_CRITICAL(&lock);
    }
}
void scanEvent(void *, esp_event_base_t, int32_t, void *data) {
    if (accepting.load()) {
        auto *ev = static_cast<wifi_event_sta_scan_done_t *>(data);
        if (ev->status) failed = true;
        else complete = true;
    }
}
bool wifiScan(bool meter) {
    wifi_scan_config_t cfg{};
    cfg.show_hidden = true;
    cfg.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    cfg.scan_time.passive = 150;
    if (meter) {
        cfg.channel = target.channel;
        cfg.bssid = target.id;
    }
    complete = false;
    lastScan = millis();
    ++attempts;
    scanPending = esp_wifi_scan_start(&cfg, false) == ESP_OK;
    return scanPending;
}
bool start(Owner who) {
    minimumHeap = ESP.getFreeHeap();
    error = "Radio initialization failed";
    ready = false;
    complete = false;
    failed = false;
    attempts = 0;
    if (who == Owner::Bluetooth) {
        advertisements.count = 0;
        advertisements.limited = false;
        esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (!(btInit = esp_bt_controller_init(&cfg) == ESP_OK)) return false;
        if (!(btEnabled = esp_bt_controller_enable(ESP_BT_MODE_BLE) == ESP_OK)) return false;
        if (!(hostInit = esp_bluedroid_init() == ESP_OK)) return false;
        if (!(hostEnabled = esp_bluedroid_enable() == ESP_OK)) return false;
        if (esp_ble_gap_register_callback(gap) != ESP_OK) return false;
        static esp_ble_scan_params_t params = {
            BLE_SCAN_TYPE_PASSIVE,
            BLE_ADDR_TYPE_PUBLIC,
            BLE_SCAN_FILTER_ALLOW_ALL,
            0x80,
            0x40,
            BLE_SCAN_DUPLICATE_DISABLE
        };
        accepting = true;
        return esp_ble_gap_set_scan_params(&params) == ESP_OK;
    }
    if (esp_netif_init() != ESP_OK) return false;
    auto rc = esp_event_loop_create_default();
    if (rc != ESP_OK && rc != ESP_ERR_INVALID_STATE) return false;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = 0;
    if (!(wifiInit = esp_wifi_init(&cfg) == ESP_OK)) return false;
    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK || esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK)
        return false;
    if (!(registered = esp_event_handler_instance_register(
                           WIFI_EVENT, WIFI_EVENT_SCAN_DONE, scanEvent, nullptr, &handler
                       ) == ESP_OK))
        return false;
    if (!(wifiStarted = esp_wifi_start() == ESP_OK)) return false;
    accepting = true;
    return wifiScan(who == Owner::Meter);
}
bool stop(Owner who) {
    accepting = false;
    bool ok = true;
    if (who == Owner::Bluetooth) {
        if (hostEnabled) {
            esp_ble_gap_stop_scanning();
            ok &= esp_bluedroid_disable() == ESP_OK;
            hostEnabled = false;
        }
        if (hostInit) {
            ok &= esp_bluedroid_deinit() == ESP_OK;
            hostInit = false;
        }
        if (btEnabled) {
            ok &= esp_bt_controller_disable() == ESP_OK;
            btEnabled = false;
        }
        if (btInit) {
            ok &= esp_bt_controller_deinit() == ESP_OK;
            btInit = false;
        }
        portENTER_CRITICAL(&lock);
        results = advertisements;
        portEXIT_CRITICAL(&lock);
    } else {
        if (scanPending) esp_wifi_scan_stop();
        scanPending = false;
        if (registered) {
            ok &= esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, handler) == ESP_OK;
            registered = false;
        }
        if (wifiStarted) {
            esp_wifi_clear_ap_list();
            ok &= esp_wifi_stop() == ESP_OK;
            wifiStarted = false;
        }
        if (wifiInit) {
            ok &= esp_wifi_deinit() == ESP_OK;
            wifiInit = false;
        }
    }
    if (!ok) error = "Shutdown failed: reboot";
    return ok;
}
void recordScanError(Owner who) {
    if ((who != Owner::Explorer && who != Owner::Channels) || !ws::recorder.wants(ws::Channels)) return;
    for (unsigned i = 0; i < 14; ++i)
        ws::recorder.observe(ws::Metric(ws::Channel1 + i), ws::Origin::Device, 0, true);
}
const Driver driver = {start, stop};
} // namespace
bool begin(Owner who, const Record *ap) {
    if (session.owner != Owner::None || session.fault) return false;
    if (who == Owner::Meter) {
        if (!ap) return false;
        target = *ap;
        samples = Samples{};
    } else {
        results.count = 0;
        results.limited = false;
    }
    bool ok = session.begin(who, millis(), who == Owner::Meter ? 38000 : 12000, driver);
    if (!ok) recordScanError(who);
    return ok;
}
void poll() {
    Owner who = session.owner;
    if (who == Owner::None) return;
    minimumHeap = std::min(minimumHeap, (unsigned)ESP.getFreeHeap());
    session.tick(millis(), driver);
    if (session.owner == Owner::None) {
        recordScanError(who);
        return;
    }
    if (who == Owner::Bluetooth && ready.exchange(false) && esp_ble_gap_start_scanning(5) != ESP_OK)
        failed = true;
    if (failed.load()) {
        recordScanError(who);
        error = "Scan failed";
        session.end(who, Phase::Error, driver);
        return;
    }
    if (complete.exchange(false)) {
        if (who != Owner::Bluetooth) {
            scanPending = false;
            uint16_t total = 0;
            if (esp_wifi_scan_get_ap_num(&total) != ESP_OK) {
                failed = true;
                return;
            }
            static wifi_ap_record_t records[32];
            uint16_t n = std::min<unsigned>(total, 32);
            if (n && esp_wifi_scan_get_ap_records(&n, records) != ESP_OK) {
                failed = true;
                return;
            }
            results.limited |= total > 32;
            for (unsigned i = 0; i < n; ++i) {
                auto &w = records[i];
                if (who == Owner::Meter) {
                    if (!memcmp(w.bssid, target.id, 6)) {
                        samples.add(w.rssi);
                        break;
                    }
                    continue;
                }
                Record r{};
                memcpy(r.id, w.bssid, 6);
                r.channel = w.primary;
                r.security = w.authmode;
                r.rssi = w.rssi;
                nameCopy(r.name, w.ssid, strnlen((char *)w.ssid, 32));
                results.add(r);
            }
            if ((who == Owner::Explorer || who == Owner::Channels) && ws::recorder.wants(ws::Channels)) {
                unsigned counts[14]{};
                for (unsigned i = 0; i < results.count; ++i) {
                    unsigned ch = results.rows[i].channel;
                    if (ch >= 1 && ch <= 14) ++counts[ch - 1];
                }
                for (unsigned i = 0; i < 14; ++i)
                    ws::recorder.observe(ws::Metric(ws::Channel1 + i), ws::Origin::Device, counts[i]);
            }
            memset(records, 0, n * sizeof(records[0]));
            if (who == Owner::Meter && attempts < 30) return;
        }
        session.end(who, Phase::Done, driver);
        if (who != Owner::Meter) results.sort(false);
        return;
    }
    if (who == Owner::Meter && !scanPending && millis() - lastScan >= 1000 && !wifiScan(true)) failed = true;
}
void cancel() { session.end(session.owner, Phase::Cancelled, driver); }
void forget() {
    cancel();
    results = Results{};
    advertisements = Results{};
    samples = Samples{};
    target = Record{};
    if (!session.fault) session.phase = Phase::Idle;
}
const char *securityName(unsigned s) {
    static const char *names[] = {
        "Open", "WEP", "WPA", "WPA2", "WPA/WPA2", "Enterprise", "WPA3", "WPA2/WPA3", "WAPI", "WPA3 Enterprise"
    };
    return s < sizeof(names) / sizeof(*names) ? names[s] : "Other";
}
} // namespace air
