#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
set -eu
cd "$(dirname "$0")/.."
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
flags=
case "${1:-}" in
    --sanitize) flags="-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer" ;;
    --update-previews|"") ;;
    *) echo "Usage: sh tests/run.sh [--sanitize | --update-previews]" >&2; exit 2 ;;
esac
check() {
    test_name=$1
    shift
    "${CXX:-c++}" $flags -std=c++11 -Wall -Wextra -Werror -Isrc "tests/$test_name.cpp" "$@" -o "$out/$test_name"
    "$out/$test_name" > "$out/$test_name.out"
    printf '%s passed\n' "$test_name"
}
check bench_transport_test src/modules.cpp src/bench.cpp src/reports.cpp src/bench_offline.cpp
check bench_test -Itests/lab_bus src/profiles.cpp src/modules.cpp src/bench.cpp src/bench_offline.cpp src/bench_reference.cpp src/bench_ui.cpp src/reports.cpp src/preferences.cpp
check bench_preview -Itests/lab_bus src/profiles.cpp src/modules.cpp src/bench.cpp src/bench_offline.cpp src/bench_reference.cpp src/bench_ui.cpp src/reports.cpp src/preferences.cpp
check navigation_test src/navigation.cpp src/registry.cpp
check modules_test src/navigation.cpp src/registry.cpp src/modules.cpp
check wireless_test src/wireless_model.cpp
check signal_test src/reports.cpp src/bench_offline.cpp src/signal.cpp
check u219_test -DBLACKBOX_BENCH_CONFIRMED_U219=1 -Itests/gpio_fixture src/reports.cpp src/bench_offline.cpp src/signal.cpp src/u219.cpp src/modules.cpp
check lab_test src/lab.cpp src/ndef.cpp src/modules.cpp
check lab_profiles_test -Itests/lab_bus src/profiles.cpp src/modules.cpp
check lab_driver_test -Itests/lab_bus -Itests/gpio_fixture src/nfc_reader.cpp src/ndef.cpp src/ir_emitter.cpp
check wireless_preview src/wireless_model.cpp src/wireless_ui.cpp
check signal_preview src/preferences.cpp src/reports.cpp src/bench_offline.cpp src/signal.cpp src/signal_ui.cpp src/modules.cpp
check lab_preview src/preferences.cpp src/lab.cpp src/ndef.cpp src/modules.cpp src/lab_ui.cpp
check workspace_test src/storage.cpp src/preferences.cpp src/search.cpp src/registry.cpp src/reports.cpp src/bench_offline.cpp src/bench_reference.cpp
check preferences_test -Itests/nvs_fixture src/settings_nvs.cpp src/preferences.cpp
check workspace_preview src/modules.cpp src/files_ui.cpp src/workspace_ui.cpp src/workspace_view.cpp src/storage.cpp src/search.cpp src/preferences.cpp src/registry.cpp src/reports.cpp src/bench_offline.cpp src/bench_reference.cpp
check keyboard_test -Itests/keyboard_fixture src/input.cpp
check nvs_guard_test -Itests/rc_platform src/nvs_guard.cpp
check rc_preview -Itests/rc_platform -Itests/lab_bus src/navigation.cpp src/registry.cpp src/health.cpp src/display.cpp src/modules.cpp src/profiles.cpp src/bench.cpp src/bench_offline.cpp src/bench_reference.cpp src/bench_ui.cpp src/files_ui.cpp src/workspace_ui.cpp src/workspace_view.cpp src/storage.cpp src/preferences.cpp src/search.cpp src/reports.cpp src/signal.cpp src/signal_ui.cpp src/lab.cpp src/lab_ui.cpp src/ndef.cpp src/wireless_model.cpp src/wireless_ui.cpp
if [ "${1:-}" = "--update-previews" ]; then
    cp "$out/rc_preview.out" ../docs/rc-preview.svg
    cp "$out/workspace_preview.out" ../docs/workspace-preview.svg
fi
cmp "$out/rc_preview.out" ../docs/rc-preview.svg
cmp "$out/workspace_preview.out" ../docs/workspace-preview.svg
cmp "$out/wireless_preview.out" ../docs/wireless-preview.svg
cmp "$out/signal_preview.out" ../docs/signal-preview.svg
cmp "$out/lab_preview.out" ../docs/nfc-ir-preview.svg
cmp "$out/bench_preview.out" ../docs/hardware-lab-preview.svg
