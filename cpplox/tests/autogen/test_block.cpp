#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Block : public End2EndTest
{
};

TEST_F(Block, empty)
{
  run(R";-](
{
}  // By itself.

// In a statement.
if (true) {
}
if (false) {
} else {
}

print "ok";  // expect: ok
);-]");
}

TEST_F(Block, scope)
{
  run(R";-](
var a = "outer";

{
  var a = "inner";
  print a;  // expect: inner
}

print a;  // expect: outer
);-]");
}
