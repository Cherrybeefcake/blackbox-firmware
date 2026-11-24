# Task 002 build record

Verified GitHub commit 2b6ba4d490bfcf94ce2574c6154537dbd5838c3d matches the upstream 1.17 checkout recorded by CI. The review workspace contains documentation only. No local dependency resolution, compiler run or binary generation took place. The unchanged full image includes autonomous wireless attack functionality. Producing that image falls outside this engineering work.

**Upstream release evidence.** The Cardputer job succeeded with warnings on November 12, 2026, in 495.84 seconds. CI set release version metadata and disabled RAM logging. A development configuration uses different flags. [Job and full diagnostics](https://github.com/BruceDevices/firmware/actions/runs/43319892630/job/131537484588)

| Measurement | Upstream release |
|---|---:|
| ELF-based image estimate | 4,498,549 bytes |
| Padded app length, inferred from merged length minus 65,536-byte app offset | 4,498,720 bytes |
| Factory application capacity | 5,111,808 bytes |
| Remaining partition margin | 613,088 bytes (about 12%) |
| Merged firmware file | 4,564,256 bytes |
| Static RAM | 129,396 / 327,680 bytes (39.5%) |

GitHub publishes the merged release file SHA-256 as `b5609a1f96b07a6de6c3402b4b932467d1147277b6bb1d587ae1780e3f3146c6`. This is release metadata. A standalone application-file digest and locally computed binary hashes are unavailable. [Release metadata](https://api.github.com/repos/BruceDevices/firmware/releases/tags/1.17) · [Partition table](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/custom_8Mb.csv)

**Toolchain observed in CI.** Python 3.13.18, PlatformIO Core 6.2.0, pioarduino platform 55.03.39, Arduino framework 3.3.9, framework libraries 5.5.4+sha.735507283d and Xtensa compiler package 14.2.0+20260121. These are the versions used by the successful job. The source also calls for a host C compiler to generate interpreter headers.

**Hook review.** Submodule initialization can fetch dependencies. Patch hooks change installed framework archives and library sources. Asset generation can submit web resources to an external minifier and generate timestamped gzip content. Other hooks generate current-year and interpreter headers, prepare link-time optimization and merge the final image. Floating dependencies, restored caches and generated content limit reproducibility from the source SHA alone. [Hook declarations](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/platformio.ini#L194-L248) · [Asset generator](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/patch.py#L94-L203)

**Resolved library graph from the upstream job.** Entries without a version below had no version printed. Vendored components are tied to the source tree. This inventory reflects historical resolution.

```text
|-- WireGuard-ESP32 @ 0.1.5+sha.90aa6b4
|-- IRremoteESP8266 @ 2.8.6+sha.22cd38c
|-- Time @ 1.6.1
|-- LibSSH-ESP32 @ 5.10.0
|-- Adafruit BusIO @ 1.17.2-bruce.1
|-- Adafruit PN532 @ 1.3.3-bruce.1
|-- BER-TLV @ 0.0.0+20261112225401.sha.bd61b70
|-- RFID_MFRC522v2 @ 2.0.6+sha.a6977df
|-- ESP Chameleon Ultra @ 0.0.2+sha.f66f354
|-- ESP Amiibolink @ 0.0.2+sha.fc26412
|-- SimpleCLI @ 1.1.4+sha.b23d377
|-- ESP PN32BLE @ 0.0.2+sha.542f121
|-- ESP PN532 UART @ 0.0.2+sha.879e586
|-- ESP PN532Killer @ 0.0.7+sha.71bf110
|-- NTPClient @ 3.2.1
|-- ESP32Time @ 2.0.6
|-- FFat @ 3.3.9
|-- ArduinoJson @ 7.4.3
|-- ESP8266Audio @ 2.4.1
|-- ESP8266SAM @ 1.1.0
|-- TinyGPSPlus @ 1.1.0
|-- FFT @ 0.0.1
|-- NimBLE-Arduino @ 2.5.1
|-- RF24 @ 1.4.11
|-- Adafruit Si4713 Library @ 1.2.3
|-- JPEGDecoder @ 2.0.0
|-- SmartRC-CC1101-Driver-Lib @ 2.5.7+sha.7bba521
|-- mquickjs @ 0.0.6+sha.169b5ea
|-- OneWire @ 2.3.8+sha.42e045e
|-- ESP32-PSRamFS @ 1.0.6+sha.5ace2e6
|-- AnimatedGIF @ 2.2.3
|-- PNGdec @ 1.1.6
|-- ESPAsyncWebServer @ 3.12.1
|-- FastLED @ 3.10.3
|-- RadioLib @ 7.8.1
|-- ST25R3916-fork @ 1.1.0+sha.0c8e00f
|-- NFC-RFAL-fork @ 1.0.1+sha.7bde458
|-- Adafruit TCA8418 @ 1.0.2
|-- utility
|-- Wire @ 3.3.9
|-- WiFi @ 3.3.9
|-- BruceHAL @ 7.3.1
|-- HTTPClient @ 3.3.9
|-- LittleFS @ 3.3.9
|-- CYD-touch
|-- Preferences @ 3.3.9
|-- ESP32_Button @ 0.0.1
|-- RotaryEncoder @ 1.5.3
|-- DisplayDrivers @ 0.0.2
|-- SPI @ 3.3.9
|-- SensorLib @ 0.4.1
|-- XPowersLib @ 0.3.3
|-- Ethernet @ 3.3.9
|-- Networking @ 3.3.9
|-- FS @ 3.3.9
|-- Bad_Usb_Lib
|-- TFT_eSPI QRcode @ 1.0.0
|-- AsyncTCP @ 3.5.0
|-- DNSServer @ 3.3.9
|-- BruceWifite @ 0.8.1-bruce
|-- RTC
|-- ESPmDNS @ 3.3.9
|-- PN532_SRIX
|-- NetworkClientSecure @ 3.3.9
|-- SPIFFS @ 3.3.9
```

**Compiler diagnostics.** The job succeeded with warnings including deprecated interfaces, narrowing conversions and link-time diagnostics. All matching historical warning/error lines follow. Full multiline context is available in the job log above. There are no local compiler diagnostics.

```text
.pio/libdeps/m5stack-cardputer/IRremoteESP8266/src/IRrecv.cpp:222:10: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/libssh_esp32_compat.c:13:38: warning: initialization discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/libssh_esp32_compat.c:45:50: warning: initialization discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/libssh_esp32_compat.c:46:44: warning: initialization discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/libssh_esp32_compat.c:46:60: warning: initialization discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/libssh_esp32_compat.c:22:37: warning: initialization discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/libssh_esp32_compat.c:47:46: warning: initialization discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/scp.c:110:5: warning: 'ssh_scp_free' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/scp.c:303:9: warning: 'ssh_scp_close' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/scp.c:577:5: warning: 'ssh_scp_push_file64' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/LibSSH-ESP32/src/scp.c:1039:9: warning: 'ssh_scp_accept_request' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:446:27: warning: narrowing conversion of '(((int)slot) - 1)' from 'int' to 'uint8_t' {aka 'unsigned char'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:457:27: warning: narrowing conversion of '(((int)slot) - 1)' from 'int' to 'uint8_t' {aka 'unsigned char'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:468:27: warning: narrowing conversion of '(((int)slot) - 1)' from 'int' to 'uint8_t' {aka 'unsigned char'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:468:46: warning: narrowing conversion of '((((int)tagType) >> 8) & 255)' from 'int' to 'uint8_t' {aka 'unsigned char'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:468:62: warning: narrowing conversion of '(((int)tagType) & 255)' from 'int' to 'uint8_t' {aka 'unsigned char'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:708:32: warning: narrowing conversion of 'length' from 'size_t' {aka 'unsigned int'} to 'uint8_t' {aka 'unsigned char'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP8266Audio/src/AudioOutputPDM.cpp:64:63: warning: narrowing conversion of '((AudioOutputPDM*)this)->AudioOutputPDM::AdjustI2SRate(((int)((AudioOutputPDM*)this)->AudioOutputPDM::AudioOutputI2S.AudioOutputI2S::AudioOutput.AudioOutput::hertz))' from 'int' to 'uint32_t' {aka 'long unsigned int'} [-Wnarrowing]
.pio/libdeps/m5stack-cardputer/ESP8266Audio/src/libmad/synth.c:117:11: warning: "SHIFT" redefined
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:746:21: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:790:29: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:793:29: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:796:29: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:799:29: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:805:29: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/mquickjs/mquickjs.c:820:29: warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
.pio/libdeps/m5stack-cardputer/OneWire/OneWire.cpp:599:22: warning: extra tokens at end of #undef directive
.pio/libdeps/m5stack-cardputer/OneWire/OneWire.cpp:600:20: warning: extra tokens at end of #undef directive
.pio/libdeps/m5stack-cardputer/ESP32-PSRamFS/src/pfs.c:38:4: warning: #warning "No SPIRAM detected, will use heap" [-Wcpp]
.pio/libdeps/m5stack-cardputer/FastLED/src/platforms/esp/32/rmt_5/idf5_rmt.cpp:8:2: warning: #warning "FASTLED_RMT_BUILTIN_DRIVER is not supported in RMT5 and will be ignored." [-Wcpp]
.pio/libdeps/m5stack-cardputer/FastLED/src/third_party/yves/I2SClockLessLedDriveresp32s3/src/I2SClockLessLedDriveresp32s3.h:397:20: warning: 'esp_lcd_i80_bus_config_t::<unnamed union>::psram_trans_align' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/FastLED/src/third_party/yves/I2SClockLessLedDriveresp32s3/src/I2SClockLessLedDriveresp32s3.h:397:20: warning: 'esp_lcd_i80_bus_config_t::<unnamed union>::psram_trans_align' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/FastLED/src/third_party/yves/I2SClockLessLedDriveresp32s3/src/I2SClockLessLedDriveresp32s3.h:397:20: warning: 'esp_lcd_i80_bus_config_t::<unnamed union>::psram_trans_align' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/FastLED/src/third_party/yves/I2SClockLessLedDriveresp32s3/src/I2SClockLessLedDriveresp32s3.h:398:20: warning: 'esp_lcd_i80_bus_config_t::sram_trans_align' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/FastLED/src/third_party/yves/I2SClockLessLedDriveresp32s3/src/I2SClockLessLedDriveresp32s3.h:398:20: warning: 'esp_lcd_i80_bus_config_t::sram_trans_align' is deprecated [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/FastLED/src/third_party/yves/I2SClockLessLedDriveresp32s3/src/I2SClockLessLedDriveresp32s3.h:398:20: warning: 'esp_lcd_i80_bus_config_t::sram_trans_align' is deprecated [-Wdeprecated-declarations]
lib/ESP32_Button/src/original/button_adc.c:199:13: warning: 'ADC_ATTEN_DB_11' is deprecated [-Wdeprecated-declarations]
lib/ESP32_Button/src/original/button_adc.c:204:9: warning: 'ADC_ATTEN_DB_11' is deprecated [-Wdeprecated-declarations]
lib/Bad_Usb_Lib/BleKeyboard.cpp:181:23: warning: 'void NimBLEHIDDevice::startServices()' is deprecated: Services are now started by the server when start() is called, this function is no longer needed and will be removed in a future release. [-Wdeprecated-declarations]
src/core/config.cpp:824:27: warning: comparison is always false due to limited range of data type [-Wtype-limits]
.pio/libdeps/m5stack-cardputer/PNGdec/src/PNGdec.h:217:9: warning: "INTELSHORT" redefined
.pio/libdeps/m5stack-cardputer/PNGdec/src/PNGdec.h:218:9: warning: "INTELLONG" redefined
src/core/menu_items/FileMenu.cpp:17:35: warning: implicit capture of 'this' via '[=]' is deprecated in C++20 [-Wdeprecated]
src/core/menu_items/FileMenu.cpp:23:27: warning: implicit capture of 'this' via '[=]' is deprecated in C++20 [-Wdeprecated]
src/core/mykeyboard.cpp:591:13: warning: variable 'longNextPress' set but not used [-Wunused-but-set-variable]
src/core/mykeyboard.cpp:592:13: warning: variable 'longPrevPress' set but not used [-Wunused-but-set-variable]
src/core/mykeyboard.cpp:591:13: warning: variable 'longNextPress' set but not used [-Wunused-but-set-variable]
src/core/mykeyboard.cpp:592:13: warning: variable 'longPrevPress' set but not used [-Wunused-but-set-variable]
src/core/mykeyboard.cpp:591:13: warning: variable 'longNextPress' set but not used [-Wunused-but-set-variable]
src/core/mykeyboard.cpp:592:13: warning: variable 'longPrevPress' set but not used [-Wunused-but-set-variable]
src/core/serial_commands/storage_commands.cpp:269:10: warning: variable 'headerReceived' set but not used [-Wunused-but-set-variable]
src/hal/inputs/touch.cpp:19:13: warning: 'void panelSize(int16_t&, int16_t&)' defined but not used [-Wunused-function]
src/modules/badusb_ble/ducky_typer.cpp:526:1: warning: 'void sendQueuedKeys(HIDInterface*, bool&, const std::vector<QueuedHIDKey>&)' defined but not used [-Wunused-function]
src/modules/badusb_ble/ducky_typer.cpp:509:1: warning: 'void keyboardSectionAction(HIDInterface*, bool&, std::vector<QueuedHIDKey>&)' defined but not used [-Wunused-function]
src/modules/badusb_ble/ducky_typer.cpp:481:13: warning: 'void openKeySection(HIDInterface*, const char*, const KeyboardMenuKey*, size_t, bool&, std::vector<QueuedHIDKey>&)' defined but not used [-Wunused-function]
src/modules/ble_api/services/BLESerialService.cpp:51:20: warning: 'bool NimBLEService::start()' is deprecated: NimBLEService::start() has no effect. Services are started when the server is started. [-Wdeprecated-declarations]
src/modules/ble_api/services/BatteryService.cpp:35:20: warning: 'bool NimBLEService::start()' is deprecated: NimBLEService::start() has no effect. Services are started when the server is started. [-Wdeprecated-declarations]
.pio/libdeps/m5stack-cardputer/RadioLib/src/RadioLib.h:69:4: warning: #warning "Use of USB CDC for debug output is not recommended (might stop on first sleep). Use hardware UART instead." [-Wcpp]
src/modules/others/u2f.cpp:214:13: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
src/modules/others/u2f.cpp:218:11: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
src/modules/others/u2f.cpp:684:15: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
src/modules/others/u2f.cpp:686:15: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
src/modules/rf/protocols/decoders/rf_decoder_psa.c:225:13: warning: variable 'original_buffer8' set but not used [-Wunused-but-set-variable]
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
src/modules/rf/protocols/decoders/rf_decoder_secplus_v2.cpp:47:13: warning: 'void sp2_mix_order_encode(uint8_t, uint16_t*)' defined but not used [-Wunused-function]
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
src/modules/rf/protocols/decoders/rf_decoder_somfy_keytis.cpp:47:16: warning: enumeration value 'ST_PREAMBLE_L' not handled in switch [-Wswitch]
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
src/modules/rf/protocols/decoders/rf_decoder_somfy_telis.cpp:46:16: warning: enumeration value 'ST_PREAMBLE_L' not handled in switch [-Wswitch]
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
src/modules/rf/protocols/decoders/rf_decoder_toyota_tpms.cpp:24:9: warning: variable 'sync_idx' set but not used [-Wunused-but-set-variable]
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
cc1: warning: command-line option '-Wno-overloaded-virtual=0' is valid for C++/ObjC++ but not for C
src/modules/rfid/PN532.cpp:688:18: warning: suggest braces around empty body in an 'if' statement [-Wempty-body]
src/modules/rfid/PN532KillerTools.cpp:775:20: warning: 'bool NimBLEService::start()' is deprecated: NimBLEService::start() has no effect. Services are started when the server is started. [-Wdeprecated-declarations]
src/modules/rfid/ST25R3916.cpp:1257:30: warning: comparison is always true due to limited range of data type [-Wtype-limits]
src/modules/rfid/ST25R3916.cpp:27:20: warning: 'const char* _stateStr(rfalNfcState)' defined but not used [-Wunused-function]
src/modules/wifi/channel_analyzer.cpp:30:5: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
src/modules/wifi/jam_detect.cpp:37:5: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
src/modules/wifi/jam_detect.cpp:43:58: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
src/modules/wifi/sdr.cpp:885:54: warning: format '%u' expects argument of type 'unsigned int', but argument 3 has type 'uint32_t' {aka 'long unsigned int'} [-Wformat=]
src/modules/wifi/wifi_recover.cpp:614:1: warning: 'always_inline' function might not be inlinable unless also declared 'inline' [-Wattributes]
src/modules/wifi/wifi_recover.cpp:594:1: warning: 'always_inline' function might not be inlinable unless also declared 'inline' [-Wattributes]
src/modules/wifi/wifi_recover.cpp:478:1: warning: 'always_inline' function might not be inlinable unless also declared 'inline' [-Wattributes]
src/modules/wifi/wifi_recover.cpp:444:1: warning: 'always_inline' function might not be inlinable unless also declared 'inline' [-Wattributes]
src/modules/wifi/wifi_recover.cpp:594:1: warning: 'void hmac_sha1_20(const HmacSha1Pre&, const uint8_t*, uint8_t*)' defined but not used [-Wunused-function]
src/modules/wifi/wifite.cpp:18:60: warning: using value of assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
src/modules/wifi/wifite.cpp:18:51: warning: using value of assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
src/modules/wifi/wifite.cpp:18:42: warning: using value of assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
src/modules/wifi/wifite.cpp:18:32: warning: using value of assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
src/modules/wifi/wifite.cpp:18:24: warning: using value of assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
src/modules/wifi/wifite.cpp:19:31: warning: using value of assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
src/modules/wire/logic_analyzer.cpp:418:18: warning: variable 't' set but not used [-Wunused-but-set-variable]
src/modules/wire/logic_analyzer.cpp:630:22: warning: comparison is always false due to limited range of data type [-Wtype-limits]
.pio/libdeps/m5stack-cardputer/ESP Chameleon Ultra/src/chameleonUltra.cpp:32:7: warning: virtual table of type 'struct scanCallbacks' violates one definition rule [-Wodr]
lto-wrapper: warning: using serial compilation of 72 LTRANS jobs
.pio/libdeps/m5stack-cardputer/ESP8266Audio/src/libmad/layer3.c:1746:5: warning: 'fastsdct' accessing 72 bytes in a region of size 68 [-Wstringop-overflow=]
.pio/libdeps/m5stack-cardputer/ESP8266Audio/src/libhelix-aac/sbrhuff.c:395:59: warning: writing 1 byte into a region of size 0 [-Wstringop-overflow=]
.pio/libdeps/m5stack-cardputer/ESP8266Audio/src/libhelix-aac/sbrhuff.c:389:59: warning: writing 1 byte into a region of size 0 [-Wstringop-overflow=]
Warning: DEPRECATED: 'esptool.py' is deprecated. Please use 'esptool' instead. The '.py' suffix will be removed in a future major release.
Warning: DEPRECATED: 'esptool.py' is deprecated. Please use 'esptool' instead. The '.py' suffix will be removed in a future major release.
```
