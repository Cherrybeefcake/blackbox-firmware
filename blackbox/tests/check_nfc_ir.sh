#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
set -eu
cd "$(dirname "$0")/.."
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
check() {
    test_name=$1
    shift
    "${CXX:-c++}" -std=c++11 -Wall -Wextra -Werror -Isrc "tests/$test_name.cpp" "$@" -o "$out/$test_name"
    "$out/$test_name" > "$out/$test_name.out"
    printf '%s passed\n' "$test_name"
}
check bench_transport_test src/modules.cpp src/bench.cpp
check bench_test -Itests/lab_bus src/profiles.cpp src/modules.cpp src/bench.cpp src/bench_offline.cpp src/bench_reference.cpp src/bench_ui.cpp
check bench_preview -Itests/lab_bus src/profiles.cpp src/modules.cpp src/bench.cpp src/bench_offline.cpp src/bench_reference.cpp src/bench_ui.cpp
check navigation_test src/navigation.cpp src/registry.cpp
check modules_test src/navigation.cpp src/registry.cpp src/modules.cpp
check wireless_test src/wireless_model.cpp
check signal_test src/signal.cpp
check u219_test -DBLACKBOX_BENCH_CONFIRMED_U219=1 -Itests/gpio_fixture src/signal.cpp src/u219.cpp src/modules.cpp
check lab_test src/lab.cpp src/ndef.cpp src/modules.cpp
check lab_profiles_test -Itests/lab_bus src/profiles.cpp src/modules.cpp
check lab_driver_test -Itests/lab_bus -Itests/gpio_fixture src/nfc_reader.cpp src/ndef.cpp src/ir_emitter.cpp
check wireless_preview src/wireless_model.cpp src/wireless_ui.cpp
check signal_preview src/signal.cpp src/signal_ui.cpp src/modules.cpp
check lab_preview src/lab.cpp src/ndef.cpp src/modules.cpp src/lab_ui.cpp
cmp "$out/wireless_preview.out" ../docs/wireless-preview.svg
cmp "$out/signal_preview.out" ../docs/signal-preview.svg
cmp "$out/lab_preview.out" ../docs/nfc-ir-preview.svg
cmp "$out/bench_preview.out" ../docs/hardware-lab-preview.svg
