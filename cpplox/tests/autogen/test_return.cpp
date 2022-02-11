#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Return : public End2EndTest
{
};

TEST_F(Return, after_else)
{
  run(R";-](
fun f()
{
  if (false)
    "no";
  else
    return "ok";
}

print f();  // expect: ok
);-]");
}

TEST_F(Return, after_if)
{
  run(R";-](
fun f()
{
  if (true)
    return "ok";
}

print f();  // expect: ok
);-]");
}

TEST_F(Return, after_while)
{
  run(R";-](
fun f()
{
  while (true)
    return "ok";
}

print f();  // expect: ok
);-]");
}

TEST_F(Return, at_top_level)
{
  run(R";-](
return "wat";  // Error at 'return': Can't return from top-level code.
);-]");
}

TEST_F(Return, in_function)
{
  run(R";-](
fun f()
{
  return "ok";
  print "bad";
}

print f();  // expect: ok
);-]");
}

TEST_F(Return, in_method)
{
  run(R";-](
class Foo
{
  method()
  {
    return "ok";
    print "bad";
  }
}

print Foo().method(); // expect: ok
);-]");
}

TEST_F(Return, return_nil_if_no_value)
{
  run(R";-](
fun f()
{
  return;
  print "bad";
}

print f();  // expect: nil
);-]");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}