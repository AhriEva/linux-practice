#!/bin/sh
set -eu
cd "$(dirname "$0")"
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir"
cmake --build "$build_dir" --parallel
exec "$build_dir/emulator" "$@"
