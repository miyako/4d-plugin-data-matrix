#!/bin/bash
# Build the ownership harness against a plugin source tree and run every scenario.
# usage: tests/ownership/run.sh [source-tree]   (defaults to this repository)
# Exit status is 0 only when every scenario matches its expectation for a fixed tree.
set -u
here="$(cd "$(dirname "$0")" && pwd)"
src="$(cd "${1:-$here/../..}" && pwd)"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT

sdk="$src/4D Plugin API"
flags=(-g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined
       -Wno-everything -I"$sdk" -I"$sdk/Classes" -I"$src/include")

objs=()
compile() { # file lang
    local o="$out/$(basename "$1").o"
    clang -c -x "$2" -std=c++11 "${flags[@]}" "$1" -o "$o" || exit 3
    objs+=("$o")
}
compile "$sdk/4DPluginAPI.c" objective-c++
for f in "$sdk"/Classes/*.cpp; do compile "$f" objective-c++; done
compile "$src/4DPlugin-DMTX.cpp" objective-c++
compile "$here/harness.mm" objective-c++

clang++ "${flags[@]}" "${objs[@]}" "$src/a/libdmtx.a" "$src/a/libpng16.a" "$src/a/libjpeg.a" -lz \
    -framework Cocoa -framework ImageIO -o "$out/harness" || exit 3

failed=0
run() { # expectation name args...
    local expect="$1"; shift
    local log="$out/log"
    ASAN_OPTIONS=detect_leaks=0:abort_on_error=0 "$out/harness" "$@" >"$log" 2>&1
    local status=$?
    local result=pass
    if [ $status -ne 0 ]; then
        if grep -q "AddressSanitizer" "$log"; then result=asan; else result=fail; fi
    fi
    local line; line="$(grep -E '^(PASS|FAIL)' "$log" | head -1)"
    [ "$result" = asan ] && line="$(grep -m1 'ERROR: AddressSanitizer' "$log")"
    if [ "$result" = "$expect" ]; then echo "ok   [$expect] $* :: $line"; else echo "BAD  [expected $expect, got $result] $* :: $line"; failed=1; fi
}

# controls: identical expectations on every tree
run pass ctl_encode_png
run pass ctl_encode_svg
run pass ctl_encode_jpg
run fail ctl_detector_leak
run asan ctl_detector_double_dispose
# regressions
run pass read_image
run pass read_image --compiled
run pass read_image --compiled --convert-noop
run pass read_images
run pass read_images --compiled
run pass read_images --compiled --convert-noop

exit $failed
