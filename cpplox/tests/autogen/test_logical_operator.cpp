#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Logical_operator : public End2EndTest
{
};

TEST_F(Logical_operator, and)
{
  run(R";-](
// Note: These tests implicitly depend on ints being truthy.

// Return the first non-true argument.
print false and 1;  // expect: false
print true and 1;  // expect: 1
print 1 and 2 and false;  // expect: false

// Return the last argument if all are true.
print 1 and true;  // expect: true
print 1 and 2 and 3;  // expect: 3

// Short-circuit at the first false argument.
var a = "before";
var b = "before";
(a = true) and (b = false) and (a = "bad");
print a;  // expect: true
print b;  // expect: false
);-]");
}

TEST_F(Logical_operator, and_truth)
{
  run(R";-](
// False and nil are false.
print false and "bad";  // expect: false
print nil and "bad";  // expect: nil

// Everything else is true.
print true and "ok";  // expect: ok
print 0 and "ok";  // expect: ok
print "" and "ok";  // expect: ok
);-]");
}

TEST_F(Logical_operator, or)
{
  run(R";-](
// Note: These tests implicitly depend on ints being truthy.

// Return the first true argument.
print 1 or true;  // expect: 1
print false or 1;  // expect: 1
print false or false or true;  // expect: true

// Return the last argument if all are false.
print false or false;  // expect: false
print false or false or false;  // expect: false

// Short-circuit at the first true argument.
var a = "before";
var b = "before";
(a = false) or (b = true) or (a = "bad");
print a;  // expect: false
print b;  // expect: true
);-]");
}

TEST_F(Logical_operator, or_truth)
{
  run(R";-](
// False and nil are false.
print false or "ok";  // expect: ok
print nil or "ok";  // expect: ok

// Everything else is true.
print true or "ok";  // expect: true
print 0 or "ok";  // expect: 0
print "s" or "ok";  // expect: s
);-]");
}
