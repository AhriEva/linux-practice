#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
exec ./run.sh --vfs examples/full.xml --startup examples/startup/demo.txt
