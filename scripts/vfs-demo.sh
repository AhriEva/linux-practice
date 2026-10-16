#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cmake -S . -B build
cmake --build build --parallel
for vfs in minimal files full; do
    ./build/emulator --vfs "examples/$vfs.xml" --startup examples/startup/stage3.txt
done
