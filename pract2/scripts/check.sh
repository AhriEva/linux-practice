#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir"
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
for graph in simple diamond cycle isolated; do
    "$build_dir/dependencies" --package A --repository "examples/$graph.json" \
        --test-mode --ascii --max-depth 10
done
"$build_dir/dependencies" --package A --repository examples/simple.json \
    --test-mode --ascii --max-depth 1 --exclude B
"$build_dir/dependencies" --package A --repository examples/diamond.json \
    --test-mode --order
if "$build_dir/dependencies" --package A --repository examples/cycle.json \
    --test-mode --order; then
    echo 'Ожидалась ошибка: цикл не имеет порядка загрузки' >&2
    exit 1
else
    status=$?
    if [ "$status" -ne 1 ]; then exit "$status"; fi
fi
