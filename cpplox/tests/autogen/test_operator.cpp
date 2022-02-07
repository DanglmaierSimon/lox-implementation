#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Operator : public End2EndTest
{
};

TEST_F(Operator, add_bool_nil)
{
  run(R";-](
true + nil;  // expect runtime error: Operands must be two numbers or two strings.
);-]");
}

TEST_F(Operator, add_bool_num)
{
  run(R";-](
true + 123;  // expect runtime error: Operands must be two numbers or two strings.
);-]");
}

TEST_F(Operator, add_bool_string)
{
  run(R";-](
true + "s";  // expect runtime error: Operands must be two numbers or two strings.
);-]");
}

TEST_F(Operator, add)
{
  run(R";-](
print 123 + 456;  // expect: 579
print "str" + "ing";  // expect: string
);-]");
}

TEST_F(Operator, add_nil_nil)
{
  run(R";-](
nil + nil;  // expect runtime error: Operands must be two numbers or two strings.
);-]");
}

TEST_F(Operator, add_num_nil)
{
  run(R";-](
1 + nil;  // expect runtime error: Operands must be two numbers or two strings.
);-]");
}

TEST_F(Operator, add_string_nil)
{
  run(R";-](
"s" + nil;  // expect runtime error: Operands must be two numbers or two strings.
);-]");
}

TEST_F(Operator, comparison)
{
  run(R";-](
print 1 < 2;  // expect: true
print 2 < 2;  // expect: false
print 2 < 1;  // expect: false

print 1 <= 2;  // expect: true
print 2 <= 2;  // expect: true
print 2 <= 1;  // expect: false

print 1 > 2;  // expect: false
print 2 > 2;  // expect: false
print 2 > 1;  // expect: true

print 1 >= 2;  // expect: false
print 2 >= 2;  // expect: true
print 2 >= 1;  // expect: true

// Zero and negative zero compare the same.
print 0 < -0;  // expect: false
print - 0 < 0;  // expect: false
print 0 > -0;  // expect: false
print - 0 > 0;  // expect: false
print 0 <= -0;  // expect: true
print - 0 <= 0;  // expect: true
print 0 >= -0;  // expect: true
print - 0 >= 0;  // expect: true
);-]");
}

TEST_F(Operator, divide)
{
  run(R";-](
print 8 / 2;  // expect: 4
print 12.34 / 12.34;  // expect: 1
);-]");
}

TEST_F(Operator, divide_nonnum_num)
{
  run(R";-](
"1" / 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, divide_num_nonnum)
{
  run(R";-](
1 / "1";  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, equals_class)
{
  run(R";-](
// Bound methods have identity equality.
class Foo
{
} class Bar
{
}

    print Foo
    == Foo;  // expect: true
print Foo == Bar;  // expect: false
print Bar == Foo;  // expect: false
print Bar == Bar;  // expect: true

print Foo == "Foo";  // expect: false
print Foo == nil;  // expect: false
print Foo == 123;  // expect: false
print Foo == true;  // expect: false
);-]");
}

TEST_F(Operator, equals)
{
  run(R";-](
print nil == nil;  // expect: true

print true == true;  // expect: true
print true == false;  // expect: false

print 1 == 1;  // expect: true
print 1 == 2;  // expect: false

print "str" == "str";  // expect: true
print "str" == "ing";  // expect: false

print nil == false;  // expect: false
print false == 0;  // expect: false
print 0 == "0";  // expect: false
);-]");
}

TEST_F(Operator, equals_method)
{
  run(R";-](
// Bound methods have identity equality.
class Foo
{
  method() {}
}

var foo = Foo();
var fooMethod = foo.method;

// Same bound method.
print fooMethod == fooMethod;  // expect: true

// Different closurizations.
print foo.method == foo.method;  // expect: false
);-]");
}

TEST_F(Operator, greater_nonnum_num)
{
  run(R";-](
"1" > 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, greater_num_nonnum)
{
  run(R";-](
1 > "1";  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, greater_or_equal_nonnum_num)
{
  run(R";-](
"1" >= 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, greater_or_equal_num_nonnum)
{
  run(R";-](
1 >= "1";  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, less_nonnum_num)
{
  run(R";-](
"1" < 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, less_num_nonnum)
{
  run(R";-](
1 < "1";  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, less_or_equal_nonnum_num)
{
  run(R";-](
"1" <= 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, less_or_equal_num_nonnum)
{
  run(R";-](
1 <= "1";  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, multiply)
{
  run(R";-](
print 5 * 3;  // expect: 15
print 12.34 * 0.3;  // expect: 3.702
);-]");
}

TEST_F(Operator, multiply_nonnum_num)
{
  run(R";-](
"1" * 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, multiply_num_nonnum)
{
  run(R";-](
1 * "1";  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, negate)
{
  run(R";-](
print - (3);  // expect: -3
print--(3);  // expect: 3
print-- - (3);  // expect: -3
);-]");
}

TEST_F(Operator, negate_nonnum)
{
  run(R";-](
- "s";  // expect runtime error: Operand must be a number.
);-]");
}

TEST_F(Operator, not_class)
{
  run(R";-](
class Bar
{
} print !Bar;  // expect: false
print !Bar();  // expect: false
);-]");
}

TEST_F(Operator, not_equals)
{
  run(R";-](
print nil != nil;  // expect: false

print true != true;  // expect: false
print true != false;  // expect: true

print 1 != 1;  // expect: false
print 1 != 2;  // expect: true

print "str" != "str";  // expect: false
print "str" != "ing";  // expect: true

print nil != false;  // expect: true
print false != 0;  // expect: true
print 0 != "0";  // expect: true
);-]");
}

TEST_F(Operator, not )
{
  run(R";-](
print !true;  // expect: false
print !false;  // expect: true
print !!true;  // expect: true

print !123;  // expect: false
print !0;  // expect: false

print !nil;  // expect: true

print !"";  // expect: false

fun foo() {}
print !foo;  // expect: false
);-]");
}

TEST_F(Operator, subtract)
{
  run(R";-](
print 4 - 3;  // expect: 1
print 1.2 - 1.2;  // expect: 0
);-]");
}

TEST_F(Operator, subtract_nonnum_num)
{
  run(R";-](
"1" - 1;  // expect runtime error: Operands must be numbers.
);-]");
}

TEST_F(Operator, subtract_num_nonnum)
{
  run(R";-](
1 - "1";  // expect runtime error: Operands must be numbers.
);-]");
}
