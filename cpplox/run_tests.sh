#!/usr/bin/env bash

set -euo pipefail

BUILD_CONFIGS="Debug Release"
SANITIZERS="none address undefined" 


find . -type f -name "cpplox" -delete

for CFG in $BUILD_CONFIGS; do
    for SAN in $SANITIZERS; do
        echo "configuring with config ${CFG} and sanitizer ${SAN}..."

        if [[ "$SAN" == "none" ]]; then
             cmake -G Ninja \
                 -DCMAKE_CXX_COMPILER=clang++ \
                 -DCMAKE_C_COMPILER=clang \
                 -DCMAKE_BUILD_TYPE="${CFG}" \
                 . -B build

            echo "building..."
            cmake --build build
            echo "testing..."
            python3 test.py build/src/cpplox
            echo "============================"
        else
             cmake -G Ninja \
                 -DCMAKE_CXX_COMPILER=clang++ \
                 -DCMAKE_C_COMPILER=clang \
                 -DCMAKE_BUILD_TYPE="${CFG}" \
                 -DCMAKE_CXX_FLAGS=-fsanitize="${SAN}" \
                 -DCMAKE_EXE_LINKER_FLAGS=-fsanitize="${SAN}" . -B build/"${CFG}"/"${SAN}"

            echo "building..."
            cmake --build build/"${CFG}"/"${SAN}"
            echo "testing..."
            python3 test.py build/"${CFG}"/"${SAN}"/src/cpplox
            echo "============================"
        fi

    done

    echo "============================"
done
