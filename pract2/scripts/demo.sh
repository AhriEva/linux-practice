#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
exec ./run.sh --package A --repository examples/diamond.json \
    --test-mode --ascii --max-depth 5 --order
