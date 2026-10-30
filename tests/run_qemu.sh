#!/bin/sh
set -eu

elf=${1:?usage: run_qemu.sh path/to/firmware.elf}
output=$(mktemp)
trap 'rm -f "$output"' EXIT

set +e
timeout 5s qemu-system-arm \
    -M lm3s6965evb \
    -kernel "$elf" \
    -nographic \
    -serial stdio \
    -monitor none \
    >"$output" 2>/dev/null
status=$?
set -e

if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
    cat "$output"
    exit "$status"
fi

grep -q "stellaris-uart-driver" "$output"
grep -q "\[tick 1\]" "$output"
grep -q "Echo mode" "$output"
cat "$output"
