#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class String : public End2EndTest
{
};

TEST_F(String, error_after_multiline)
{
  run(R";-](
// Tests that we correctly track the line info across multiline strings.
var a = "1
    2 3 ";

    err;  // // expect runtime error: Undefined variable 'err'.
);-]");
}

TEST_F(String, literals)
{
  run(R";-](
print "(" + "" + ")";  // expect: ()
print "a string";  // expect: a string

// Non-ASCII.
print "A~¶Þॐஃ";  // expect: A~¶Þॐஃ
);-]");
}

TEST_F(String, multiline)
{
  run(R";-](
var a = "1
    2 3 ";
    print a;
// expect: 1
// expect:     2 3 
);-]");
}

TEST_F(String, unterminated)
{
  run(R";-](
// [line 2] Error: Unterminated string.
"this string has no close quote
);-]");
}
