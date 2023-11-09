#!/usr/bin/env bash

set -euo pipefail

cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebug -G Ninja

cmake --build build

perf record build/src/cpplox "$(pwd)/tests/test/benchmark/equality.lox"
