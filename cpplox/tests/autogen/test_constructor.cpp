#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Constructor : public End2EndTest
{
};

TEST_F(Constructor, arguments)
{
  run(R";-](
class Foo
{
  init(a, b)
  {
    print "init";  // expect: init
    this.a = a;
    this.b = b;
  }
}

var foo = Foo(1, 2);
print foo.a;  // expect: 1
print foo.b;  // expect: 2
);-]");
}

TEST_F(Constructor, call_init_early_return)
{
  run(R";-](
class Foo
{
  init()
  {
    print "init";
    return;
    print "nope";
  }
}

var foo = Foo();  // expect: init
print foo.init();  // expect: init
// expect: Foo instance
);-]");
}

TEST_F(Constructor, call_init_explicitly)
{
  run(R";-](
class Foo
{
  init(arg)
  {
    print "Foo.init(" + arg + ")";
    this.field = "init";
  }
}

var foo = Foo("one");  // expect: Foo.init(one)
foo.field = "field";

var foo2 = foo.init("two");  // expect: Foo.init(two)
print foo2;  // expect: Foo instance

// Make sure init() doesn't create a fresh instance.
print foo.field;  // expect: init
);-]");
}

TEST_F(Constructor, default_arguments)
{
  run(R";-](
class Foo
{
}

var foo =
    Foo(1, 2, 3);  // expect runtime error: Expected 0 arguments but got 3.
);-]");
}

TEST_F(Constructor, default)
{
  run(R";-](
class Foo
{
}

var foo = Foo();
print foo;  // expect: Foo instance
);-]");
}

TEST_F(Constructor, early_return)
{
  run(R";-](
class Foo
{
  init()
  {
    print "init";
    return;
    print "nope";
  }
}

var foo = Foo();  // expect: init
print foo;  // expect: Foo instance
);-]");
}

TEST_F(Constructor, extra_arguments)
{
  run(R";-](
class Foo
{
  init(a, b)
  {
    this.a = a;
    this.b = b;
  }
}

var foo =
    Foo(1, 2, 3, 4);  // expect runtime error: Expected 2 arguments but got 4.
);-]");
}

TEST_F(Constructor, init_not_method)
{
  run(R";-](
class Foo
{
  init(arg)
  {
    print "Foo.init(" + arg + ")";
    this.field = "init";
  }
}

fun init()
{
  print "not initializer";
}

init();  // expect: not initializer
);-]");
}

TEST_F(Constructor, missing_arguments)
{
  run(R";-](
class Foo
{
  init(a, b) {}
}

var foo = Foo(1);  // expect runtime error: Expected 2 arguments but got 1.
);-]");
}

TEST_F(Constructor, return_in_nested_function)
{
  run(R";-](
class Foo
{
  init()
  {
    fun init()
    {
      return "bar";
    }
    print init();  // expect: bar
  }
}

print Foo(); // expect: Foo instance
);-]");
}

TEST_F(Constructor, return_value)
{
  run(R";-](
class Foo
{
  init()
  {
    return "result";  // Error at 'return': Can't return a value from an initializer.
  }
}
);-]");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}