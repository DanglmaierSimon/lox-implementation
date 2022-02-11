#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Misc : public End2EndTest
{
};

TEST_F(Misc, empty_file)
{
  run(R";-](

);-]");
}

TEST_F(Misc, precedence)
{
  run(R";-](
// * has higher precedence than +.
print 2 + 3 * 4;  // expect: 14

// * has higher precedence than -.
print 20 - 3 * 4;  // expect: 8

// / has higher precedence than +.
print 2 + 6 / 3;  // expect: 4

// / has higher precedence than -.
print 2 - 6 / 3;  // expect: 0

// < has higher precedence than ==.
print false == 2 < 1;  // expect: true

// > has higher precedence than ==.
print false == 1 > 2;  // expect: true

// <= has higher precedence than ==.
print false == 2 <= 1;  // expect: true

// >= has higher precedence than ==.
print false == 1 >= 2;  // expect: true

// 1 - 1 is not space-sensitive.
print 1 - 1;  // expect: 0
print 1 - 1;  // expect: 0
print 1 - 1;  // expect: 0
print 1 - 1;  // expect: 0

// Using () for grouping.
print(2 * (6 - (2 + 2)));  // expect: 4
);-]");
}

TEST_F(Misc, unexpected_character)
{
  run(R";-](
// [line 3] Error: Unexpected character.
foo(a | b);
);-]");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}