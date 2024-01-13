#!/usr/bin/env bash

set -euo pipefail

BUILD_CONFIGS="Debug Release"
SANITIZERS="undefined address"

for CFG in $BUILD_CONFIGS; do
    for SAN in $SANITIZERS; do
        echo "cleaning build dir..."
        cmake --build build --target clean

        echo "configuring with config ${CFG} and sanitizer ${SAN}..."
        cmake -G Ninja -DCMAKE_CXX_COMPILER=clang++-13 -DCMAKE_C_COMPILER=clang-13 -DCMAKE_BUILD_TYPE="${CFG}" -DCMAKE_CXX_FLAGS=-fsanitize="$SAN" -DCMAKE_EXE_LINKER_FLAGS=-fsanitize="$SAN" . -B build

        echo "building..."
        cmake --build build

        echo "testing..."
        ctest -j"$(nproc)" --output-on-failure --test-dir build
        echo "============================"
    done

    echo "============================"
done
