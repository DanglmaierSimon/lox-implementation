#!/usr/bin/env bash

set -euo pipefail

echo "configure"

cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebug -G Ninja

echo "build"

cmake --build build

echo "record"

# perf record -o /home/simon/playground/lox-implementation/cpplox/perf.data --call-graph dwarf --aio --sample-cpu /home/simon/playground/lox-implementation/cpplox/build/src/cpplox /home/simon/playground/lox-implementation/cpplox/tests/test/benchmark/equality.lox

# perf record build/src/cpplox "$(pwd)/tests/test/benchmark/equality.lox"

for f in "$(pwd)"/tests/test/benchmark/*.lox; do
    echo "running benchmark $(basename "$f")"

    OUTPUT_FILE="$(basename "$f").perf.data"
    BINARY=$(pwd)/build/src/cpplox

    perf record -o "${OUTPUT_FILE}" --call-graph dwarf --aio --sample-cpu "${BINARY}" "$f"

done
