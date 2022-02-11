#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class If : public End2EndTest
{
};

TEST_F(If, class_in_else)
{
  run(R";-](
// [line 2] Error at 'class': Expect expression.
if (true)
  "ok";
else
  class Foo
  {
  }
);-]");
}

TEST_F(If, class_in_then)
{
  run(R";-](
// [line 2] Error at 'class': Expect expression.
if (true)
  class Foo
  {
}
);-]");
}

TEST_F(If, dangling_else)
{
  run(R";-](
// A dangling else binds to the right-most if.
if (true)
  if (false)
    print "bad";
  else
    print "good";  // expect: good
if (false)
  if (true)
    print "bad";
  else
    print "bad";
);-]");
}

TEST_F(If, else)
{
  run(R";-](
// Evaluate the 'else' expression if the condition is false.
if (true)
  print "good";
else
  print "bad";  // expect: good
if (false)
  print "bad";
else
  print "good";  // expect: good

// Allow block body.
if (false)
  nil;
else {
  print "block";
}  // expect: block
);-]");
}

TEST_F(If, fun_in_else)
{
  run(R";-](
// [line 2] Error at 'fun': Expect expression.
if (true)
  "ok";
else
  fun foo() {}
);-]");
}

TEST_F(If, fun_in_then)
{
  run(R";-](
// [line 2] Error at 'fun': Expect expression.
if (true)
  fun foo() {}
);-]");
}

TEST_F(If, if)
{
  run(R";-](
// Evaluate the 'then' expression if the condition is true.
if (true)
  print "good";  // expect: good
if (false)
  print "bad";

// Allow block body.
if (true) {
  print "block";
}  // expect: block

// Assignment in if condition.
var a = false;
if (a = true)
  print a;  // expect: true
);-]");
}

TEST_F(If, truth)
{
  run(R";-](
// False and nil are false.
if (false)
  print "bad";
else
  print "false";  // expect: false
if (nil)
  print "bad";
else
  print "nil";  // expect: nil

// Everything else is true.
if (true)
  print true;  // expect: true
if (0)
  print 0;  // expect: 0
if ("")
  print "empty";  // expect: empty
);-]");
}

TEST_F(If, var_in_else)
{
  run(R";-](
// [line 2] Error at 'var': Expect expression.
if (true)
  "ok";
else
  var foo;
);-]");
}

TEST_F(If, var_in_then)
{
  run(R";-](
// [line 2] Error at 'var': Expect expression.
if (true)
  var foo;
);-]");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}