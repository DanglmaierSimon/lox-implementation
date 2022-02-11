#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Number : public End2EndTest
{
};

TEST_F(Number, decimal_point_at_eof)
{
  run(R";-](
// [line 2] Error at end: Expect property name after '.'.
123.
);-]");
}

TEST_F(Number, leading_dot)
{
  run(R";-](
// [line 2] Error at '.': Expect expression.
.123;
);-]");
}

TEST_F(Number, literals)
{
  run(R";-](
print 123;  // expect: 123
print 987654;  // expect: 987654
print 0;  // expect: 0
print - 0;  // expect: -0

print 123.456;  // expect: 123.456
print - 0.001;  // expect: -0.001
);-]");
}

TEST_F(Number, nan_equality)
{
  run(R";-](
var nan = 0 / 0;

print nan == 0;  // expect: false
print nan != 1;  // expect: true

// NaN is not equal to self.
print nan == nan;  // expect: false
print nan != nan;  // expect: true
);-]");
}

TEST_F(Number, trailing_dot)
{
  run(R";-](
// [line 2] Error at ';': Expect property name after '.'.
123.;
);-]");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}