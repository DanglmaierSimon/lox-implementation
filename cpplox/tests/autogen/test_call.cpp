#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Call : public End2EndTest
{
};

TEST_F(Call, bool)
{
  run(R";-](
true();  // expect runtime error: Can only call functions and classes.
);-]");
}

TEST_F(Call, nil)
{
  run(R";-](
nil();  // expect runtime error: Can only call functions and classes.
);-]");
}

TEST_F(Call, num)
{
  run(R";-](
123();  // expect runtime error: Can only call functions and classes.
);-]");
}

TEST_F(Call, object)
{
  run(R";-](
class Foo
{
}

var foo = Foo();
foo();  // expect runtime error: Can only call functions and classes.
);-]");
}

TEST_F(Call, string)
{
  run(R";-](
"str"();  // expect runtime error: Can only call functions and classes.
);-]");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}