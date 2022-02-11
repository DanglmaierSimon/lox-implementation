#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Class : public End2EndTest
{
};

TEST_F(Class, empty)
{
  run(R";-](
class Foo
{
}

print Foo;  // expect: Foo
);-]");
}

TEST_F(Class, inherited_method)
{
  run(R";-](
class Foo
{
  inFoo()
  {
    print "in foo";
  }
}

    class Bar
    < Foo
{
  inBar()
  {
    print "in bar";
  }
}

    class Baz
    < Bar
{
  inBaz()
  {
    print "in baz";
  }
}

var baz = Baz();
baz.inFoo();  // expect: in foo
baz.inBar();  // expect: in bar
baz.inBaz();  // expect: in baz
);-]");
}

TEST_F(Class, inherit_self)
{
  run(R";-](
class Foo < Foo
{
}  // Error at 'Foo': A class can't inherit from itself.
);-]");
}

TEST_F(Class, local_inherit_other)
{
  run(R";-](
class A
{
}

fun f()
{
  class B < A
  {
  } return B;
}

print f();  // expect: B
);-]");
}

TEST_F(Class, local_inherit_self)
{
  run(R";-](
{
  class Foo < Foo
  {
  }  // Error at 'Foo': A class can't inherit from itself.
}
// [line 5] Error at end: Expect '}' after block.
);-]");
}

TEST_F(Class, local_reference_self)
{
  run(R";-](
{
  class Foo
  {
    returnSelf()
    {
      return Foo;
    }
  }

  print
  Foo()
      .returnSelf();  // expect: Foo
}
);-]");
}

TEST_F(Class, reference_self)
{
  run(R";-](
class Foo
{
  returnSelf()
  {
    return Foo;
  }
}

print Foo().returnSelf(); // expect: Foo
);-]");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}