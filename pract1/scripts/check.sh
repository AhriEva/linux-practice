#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir"
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
for vfs in minimal files full; do
    "$build_dir/emulator" --headless --vfs "examples/$vfs.xml" \
        --startup examples/startup/exit.txt
done
"$build_dir/emulator" --headless --vfs examples/full.xml \
    --startup examples/startup/demo.txt
for stage in 2 3 4 5; do
    if "$build_dir/emulator" --headless --vfs examples/full.xml \
        --startup "examples/startup/stage$stage.txt"; then
        echo "Ожидались ошибки в сценарии этапа $stage" >&2
        exit 1
    else
        status=$?
        if [ "$status" -ne 1 ]; then exit "$status"; fi
    fi
done
