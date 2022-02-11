#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Regression : public End2EndTest
{
};

TEST_F(Regression, 394)
{
  run(R";-](
{
  class A
  {
  } class B < A
  {
  } print B;  // expect: B
}
);-]");
}

TEST_F(Regression, 40)
{
  run(R";-](
fun caller(g)
{
  g();
  // g should be a function, not nil.
  print g == nil;  // expect: false
}

fun callCaller()
{
  var capturedVar = "before";
  var a = "a";

  fun f()
  {
    // Commenting the next line out prevents the bug!
    capturedVar = "after";

    // Returning anything also fixes it, even nil:
    // return nil;
  }

  caller(f);
}

callCaller();
);-]");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}