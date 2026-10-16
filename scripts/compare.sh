#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
command -v cargo >/dev/null 2>&1 || {
    echo 'Для независимого сравнения нужен установленный Cargo.' >&2
    exit 1
}
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir"
cmake --build "$build_dir" --parallel
for package in itoa getrandom rand_chacha; do
    case "$package" in
        itoa) version=1.0.15 ;;
        getrandom) version=0.2.16 ;;
        rand_chacha) version=0.3.1 ;;
    esac
    "$build_dir/dependencies" --package "$package@$version" --max-depth 20 --ascii --order
    cargo tree --manifest-path "examples/cargo/$package/Cargo.toml" \
        -p "$package" --target all -e normal,build --charset ascii
done
cargo build --manifest-path examples/cargo/getrandom/Cargo.toml --jobs 1 -vv
