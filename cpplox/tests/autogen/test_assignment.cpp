#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Assignment : public End2EndTest
{
};

TEST_F(Assignment, associativity)
{
  run(R";-](
var a = "a";
var b = "b";
var c = "c";

// Assignment is right-associative.
a = b = c;
print a;  // expect: c
print b;  // expect: c
print c;  // expect: c
);-]");
}

TEST_F(Assignment, global)
{
  run(R";-](
var a = "before";
print a;  // expect: before

a = "after";
print a;  // expect: after

print a = "arg";  // expect: arg
print a;  // expect: arg
);-]");
}

TEST_F(Assignment, grouping)
{
  run(R";-](
var a = "a";
(a) = "value";  // Error at '=': Invalid assignment target.
);-]");
}

TEST_F(Assignment, infix_operator)
{
  run(R";-](
var a = "a";
var b = "b";
a + b = "value";  // Error at '=': Invalid assignment target.
);-]");
}

TEST_F(Assignment, local)
{
  run(R";-](
{
  var a = "before";
  print a;  // expect: before

  a = "after";
  print a;  // expect: after

  print a = "arg";  // expect: arg
  print a;  // expect: arg
}
);-]");
}

TEST_F(Assignment, prefix_operator)
{
  run(R";-](
var a = "a";
!a = "value";  // Error at '=': Invalid assignment target.
);-]");
}

TEST_F(Assignment, syntax)
{
  run(R";-](
// Assignment on RHS of variable.
var a = "before";
var c = a = "var";
print a;  // expect: var
print c;  // expect: var
);-]");
}

TEST_F(Assignment, to_this)
{
  run(R";-](
class Foo
{
  Foo()
  {
    this = "value";  // Error at '=': Invalid assignment target.
  }
}

Foo();
);-]");
}

TEST_F(Assignment, undefined)
{
  run(R";-](
unknown = "what";  // expect runtime error: Undefined variable 'unknown'.
);-]");
}

TEST_F(Assignment, constAssignment)
{
  run(R";-](
const a = "val1";
print a;  // expect: val1

var b = "val2";
print b; // expect: val2

b = a;
print b; // expect: val1

a = "new value"; // Error at '=': Assigning to const variable.
);-]");
}