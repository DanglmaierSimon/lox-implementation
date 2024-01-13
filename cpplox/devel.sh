#!/usr/bin/env bash

set -euo pipefail

echo "building..."
cmake --build build --target cpplox

echo "contents of test.lox"
echo "==========================="
cat build/test.lox
echo ""
echo "==========================="

echo "running..."
./build/src/cpplox ./build/test.lox
