#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Nil : public End2EndTest
{
};

TEST_F(Nil, literal)
{
  run(R";-](
print nil;  // expect: nil
);-]");
}
