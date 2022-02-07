#!/usr/bin/env bash

set -euo pipefail

cd build

cmake -DBUILD_TESTS=ON -DBUILD_BENCHMARK=OFF ..

ninja all

ctest . --output-on-failure

echo "TODO Port all tests to cmake & ctest"

cd /home/simon/playground/craftinginterpreters

dart tool/bin/test.dart clox --interpreter ../crafting-interpreters/cpp/cpplox/build/cpplox
