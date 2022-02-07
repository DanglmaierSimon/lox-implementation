#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Print : public End2EndTest
{
};

TEST_F(Print, missing_argument)
{
  run(R";-](
// [line 2] Error at ';': Expect expression.
print;
);-]");
}
