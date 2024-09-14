#!/usr/bin/env bash

set -euo pipefail

BUILD_CONFIGS="Debug Release"
SANITIZERS="undefined address"

for CFG in $BUILD_CONFIGS; do
    for SAN in $SANITIZERS; do
        echo "configuring with config ${CFG} and sanitizer ${SAN}..."
        cmake -G Ninja \
            -DCMAKE_CXX_COMPILER=clang++-13 \
            -DCMAKE_C_COMPILER=clang-13 \
            -DCMAKE_BUILD_TYPE="${CFG}" \
            -DCMAKE_CXX_FLAGS=-fsanitize="${SAN}" \
            -DCMAKE_EXE_LINKER_FLAGS=-fsanitize="${SAN}" . -B build/"${CFG}"/"${SAN}"

        echo "building..."
        cmake --build build/"${CFG}"/"${SAN}"

        echo "testing..."
        ctest -j"$(nproc)" --output-on-failure --schedule-random --test-dir build/"${CFG}"/"${SAN}"
        echo "============================"
    done

    echo "============================"
done
