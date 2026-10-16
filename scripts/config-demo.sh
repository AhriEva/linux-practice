#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cmake -S . -B build
cmake --build build --parallel
./build/emulator --vfs examples/full.xml --startup examples/startup/stage2.txt
./build/emulator --vfs examples/minimal.xml --startup examples/startup/stage2.txt
