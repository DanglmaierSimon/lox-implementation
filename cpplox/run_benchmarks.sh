#!/usr/bin/env bash

set -euo pipefail

cd build

cmake -DBUILD_BENCHMARK=ON -DBUILD_TESTS=OFF ..

ninja all

./source/benchmark/cpploxbenchmark
