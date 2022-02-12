#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Inheritance : public End2EndTest
{
};

TEST_F(Inheritance, constructor)
{
  run(R";-](
class A
{
  init(param)
  {
    this.field = param;
  }

  test()
  {
    print this.field;
  }
}

    class B
    < A
{
}

var b = B("value");
b.test();  // expect: value
);-]");
}

TEST_F(Inheritance, inherit_from_function)
{
  run(R";-](
fun foo() {}

class Subclass < foo
{
}  // expect runtime error: Superclass must be a class.
);-]");
}

TEST_F(Inheritance, inherit_from_nil)
{
  run(R";-](
var Nil = nil;
class Foo < Nil
{
}  // expect runtime error: Superclass must be a class.
);-]");
}

TEST_F(Inheritance, inherit_from_number)
{
  run(R";-](
var Number = 123;
class Foo < Number
{
}  // expect runtime error: Superclass must be a class.
);-]");
}

TEST_F(Inheritance, inherit_methods)
{
  run(R";-](
class Foo
{
  methodOnFoo()
  {
    print "foo";
  }
  override()
  {
    print "foo";
  }
}

    class Bar
    < Foo
{
  methodOnBar()
  {
    print "bar";
  }
  override()
  {
    print "bar";
  }
}

var bar = Bar();
bar.methodOnFoo();  // expect: foo
bar.methodOnBar();  // expect: bar
bar.override();  // expect: bar
);-]");
}

TEST_F(Inheritance, parenthesized_superclass)
{
  run(R";-](
class Foo
{
}

    // [line 4] Error at '(': Expect superclass name.
    class Bar
    < (Foo)
{
}
);-]");
}

TEST_F(Inheritance, set_fields_from_base_class)
{
  run(R";-](
class Foo
{
  foo(a, b)
  {
    this.field1 = a;
    this.field2 = b;
  }

  fooPrint()
  {
    print this.field1;
    print this.field2;
  }
}

    class Bar
    < Foo
{
  bar(a, b)
  {
    this.field1 = a;
    this.field2 = b;
  }

  barPrint()
  {
    print this.field1;
    print this.field2;
  }
}

var bar = Bar();
bar.foo("foo 1", "foo 2");
bar.fooPrint();
// expect: foo 1
// expect: foo 2

bar.bar("bar 1", "bar 2");
bar.barPrint();
// expect: bar 1
// expect: bar 2

bar.fooPrint();
// expect: bar 1
// expect: bar 2
);-]");
}
