#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class While : public End2EndTest
{
};

TEST_F(While, class_in_body)
{
  run(R";-](
// [line 2] Error at 'class': Expect expression.
while (true)
  class Foo
  {
}
);-]");
}

TEST_F(While, closure_in_body)
{
  run(R";-](
var f1;
var f2;
var f3;

var i = 1;
while (i < 4) {
  var j = i;
  fun f()
  {
    print j;
  }

  if (j == 1)
    f1 = f;
  else if (j == 2)
    f2 = f;
  else
    f3 = f;

  i = i + 1;
}

f1();  // expect: 1
f2();  // expect: 2
f3();  // expect: 3
);-]");
}

TEST_F(While, fun_in_body)
{
  run(R";-](
// [line 2] Error at 'fun': Expect expression.
while (true)
  fun foo() {}
);-]");
}

TEST_F(While, return_closure)
{
  run(R";-](
fun f()
{
  while (true) {
    var i = "i";
    fun g()
    {
      print i;
    }
    return g;
  }
}

var h = f();
h();  // expect: i
);-]");
}

TEST_F(While, return_inside)
{
  run(R";-](
fun f()
{
  while (true) {
    var i = "i";
    return i;
  }
}

print f();
// expect: i
);-]");
}

TEST_F(While, syntax)
{
  run(R";-](
// Single-expression body.
var c = 0;
while (c < 3)
  print c = c + 1;
// expect: 1
// expect: 2
// expect: 3

// Block body.
var a = 0;
while (a < 3) {
  print a;
  a = a + 1;
}
// expect: 0
// expect: 1
// expect: 2

// Statement bodies.
while (false)
  if (true)
    1;
  else
    2;
while (false)
  while (true)
    1;
while (false)
  for (;;)
    1;
);-]");
}

TEST_F(While, var_in_body)
{
  run(R";-](
// [line 2] Error at 'var': Expect expression.
while (true)
  var foo;
);-]");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}