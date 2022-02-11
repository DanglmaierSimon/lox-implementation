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

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}