#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir"
cmake --build "$build_dir" --parallel
for package in itoa@1.0.15 getrandom@0.2.16 rand_chacha@0.3.1; do
    "$build_dir/dependencies" --package "$package" \
        --repository https://index.crates.io/ --max-depth 20 --ascii --order
done
