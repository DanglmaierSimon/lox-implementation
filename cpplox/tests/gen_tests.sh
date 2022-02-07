#!/usr/bin/env bash

set -eou pipefail

rm ./src/*.cpp

for TEST_TYPE in ./test/*; do

    FILE_SUFFIX=$(basename "$TEST_TYPE")
    echo $FILE_SUFFIX

    if [[ $FILE_SUFFIX == "benchmark" ]]; then
        continue
    fi

    if [[ $FILE_SUFFIX == "limit" ]]; then
        continue
    fi

    FILE_NAME="./src/test_$FILE_SUFFIX.cpp"

    touch "autogen/$FILE_NAME"

    echo "#include <gtest/gtest.h>
#include \"helpers/testhelper.h\"

class ${FILE_SUFFIX^} : public End2EndTest {};


" >>"$FILE_NAME"

    for TEST in "$TEST_TYPE"/*; do

        TEST_NAME=$(basename $TEST)
        TEST_NAME=${TEST_NAME%%.*}
        echo "    $TEST_NAME"

        FILE_CONTENTS=$(cat $TEST)

        echo "TEST_F(${FILE_SUFFIX^}, $TEST_NAME)
{
    run(R\";-](
$FILE_CONTENTS
);-]\");
}

" >>"$FILE_NAME"
    done
done

find ./src/ -name "*.cpp" -exec clang-format-13 -i {} \;
