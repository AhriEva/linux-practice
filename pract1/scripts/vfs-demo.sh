#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir"
cmake --build "$build_dir" --parallel
for vfs in minimal files full; do
    "$build_dir/emulator" --vfs "examples/$vfs.xml" --startup examples/startup/stage3.txt
done
