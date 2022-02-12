#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class This : public End2EndTest
{
};

TEST_F(This, closure)
{
  run(R";-](
class Foo
{
  getClosure()
  {
    fun closure()
    {
      return this.toString();
    }
    return closure;
  }

  toString()
  {
    return "Foo";
  }
}

var closure = Foo().getClosure();
print closure();  // expect: Foo
);-]");
}

TEST_F(This, nested_class)
{
  run(R";-](
class Outer
{
  method()
  {
    print this;  // expect: Outer instance

    fun f()
    {
      print this;  // expect: Outer instance

      class Inner
      {
        method()
        {
          print this;  // expect: Inner instance
        }
      }

      Inner()
          .method();
    }
    f();
  }
}

Outer()
    .method();
);-]");
}

TEST_F(This, nested_closure)
{
  run(R";-](
class Foo
{
  getClosure()
  {
    fun f()
    {
      fun g()
      {
        fun h()
        {
          return this.toString();
        }
        return h;
      }
      return g;
    }
    return f;
  }

  toString()
  {
    return "Foo";
  }
}

var closure = Foo().getClosure();
print closure()()();  // expect: Foo
);-]");
}

TEST_F(This, this_at_top_level)
{
  run(R";-](
this;  // Error at 'this': Can't use 'this' outside of a class.
);-]");
}

TEST_F(This, this_in_method)
{
  run(R";-](
class Foo
{
  bar()
  {
    return this;
  }
  baz()
  {
    return "baz";
  }
}

print Foo().bar().baz(); // expect: baz
);-]");
}

TEST_F(This, this_in_top_level_function)
{
  run(R";-](
fun foo()
{
  this;  // Error at 'this': Can't use 'this' outside of a class.
}
);-]");
}
