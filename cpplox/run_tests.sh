#!/usr/bin/env bash

set -euo pipefail

function run {

    local TOTAL_RUNS=0
    local SUCCESFUL_RUNS=0
    local FAILED_RUNS=0

    local ok=("")
    local fail=("")

    for BUILD_TYPE in "Debug" "Release"; do
        echo "Build type: $BUILD_TYPE"

        for SANITIZER in "address" "undefined" ""; do
            TOTAL_RUNS=$((TOTAL_RUNS + 1))



            STRING="cpplox-$BUILD_TYPE"
            if [[ ! "$SANITIZER" == "" ]]; then
                STRING="$STRING-$SANITIZER"
            fi

            STRING=$(echo "$STRING" | tr '[:upper:]' '[:lower:]') # convert all to lowercase
            STRING="$STRING:latest"

            echo "$STRING"

            docker build . --rm -f "Dockerfile" -t "$STRING" --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SANITIZER=$SANITIZER
            if docker run --cap-add SYS_PTRACE "$STRING"; then
                ok+=("$STRING")
                SUCCESFUL_RUNS=$((SUCCESFUL_RUNS + 1))
            else
                fail+=("$STRING")
                FAILED_RUNS=$((FAILED_RUNS + 1))
            fi
        done
    done

    echo "Total runs: $TOTAL_RUNS"

    echo -e "\033[0;32mSUCCESSFUL RUNS:\033[0m $SUCCESFUL_RUNS"
    echo "${ok[*]}"

    echo ""

    echo -e "\033[0;31mFAILED RUNS:\033[0m $FAILED_RUNS"
    echo "${fail[*]}"
}

time run
