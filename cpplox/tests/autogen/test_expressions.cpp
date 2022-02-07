#include <gtest/gtest.h>

#include "testhelper.h"

class Expressions : public End2EndTest
{
};

TEST_F(Expressions, evaluate)
{
  run(R";-](
// Note: This is just for the expression evaluating chapter which evaluates an
// expression directly.
(5 - (3 - 1))
    + -1
      // expect: 2
);-]");
}

TEST_F(Expressions, parse)
{
  run(R";-](
// Note: This is just for the expression parsing chapter which prints the AST.
(5 - (3 - 1))
    + -1
      // expect: (+ (group (- 5.0 (group (- 3.0 1.0)))) (- 1.0))
);-]");
}
