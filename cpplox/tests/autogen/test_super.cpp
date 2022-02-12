#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Super : public End2EndTest
{
};

TEST_F(Super, bound_method)
{
  run(R";-](
class A
{
  method(arg)
  {
    print "A.method(" + arg + ")";
  }
}

    class B
    < A
{
  getClosure()
  {
    return super.method;
  }

  method(arg)
  {
    print "B.method(" + arg + ")";
  }
}

var closure = B().getClosure();
closure("arg");  // expect: A.method(arg)
);-]");
}

TEST_F(Super, call_other_method)
{
  run(R";-](
class Base
{
  foo()
  {
    print "Base.foo()";
  }
}

    class Derived
    < Base
{
  bar()
  {
    print "Derived.bar()";
    super.foo();
  }
}

Derived()
    .bar();
// expect: Derived.bar()
// expect: Base.foo()
);-]");
}

TEST_F(Super, call_same_method)
{
  run(R";-](
class Base
{
  foo()
  {
    print "Base.foo()";
  }
}

    class Derived
    < Base
{
  foo()
  {
    print "Derived.foo()";
    super.foo();
  }
}

Derived()
    .foo();
// expect: Derived.foo()
// expect: Base.foo()
);-]");
}

TEST_F(Super, closure)
{
  run(R";-](
class Base
{
  toString()
  {
    return "Base";
  }
}

    class Derived
    < Base
{
  getClosure()
  {
    fun closure()
    {
      return super.toString();
    }
    return closure;
  }

  toString()
  {
    return "Derived";
  }
}

var closure = Derived().getClosure();
print closure();  // expect: Base
);-]");
}

TEST_F(Super, constructor)
{
  run(R";-](
class Base
{
  init(a, b)
  {
    print "Base.init(" + a + ", " + b + ")";
  }
}

    class Derived
    < Base
{
  init()
  {
    print "Derived.init()";
    super.init("a", "b");
  }
}

Derived();
// expect: Derived.init()
// expect: Base.init(a, b)
);-]");
}

TEST_F(Super, extra_arguments)
{
  run(R";-](
class Base
{
  foo(a, b)
  {
    print "Base.foo(" + a + ", " + b + ")";
  }
}

    class Derived
    < Base
{
  foo()
  {
    print "Derived.foo()";  // expect: Derived.foo()
    super.foo("a",
              "b",
              "c",
              "d");  // expect runtime error: Expected 2 arguments but got 4.
  }
}

Derived()
    .foo();
);-]");
}

TEST_F(Super, indirectly_inherited)
{
  run(R";-](
class A
{
  foo()
  {
    print "A.foo()";
  }
}

    class B
    < A
{
}

    class C
    < B
{
  foo()
  {
    print "C.foo()";
    super.foo();
  }
}

C()
    .foo();
// expect: C.foo()
// expect: A.foo()
);-]");
}

TEST_F(Super, missing_arguments)
{
  run(R";-](
class Base
{
  foo(a, b)
  {
    print "Base.foo(" + a + ", " + b + ")";
  }
}

    class Derived
    < Base
{
  foo()
  {
    super.foo(1);  // expect runtime error: Expected 2 arguments but got 1.
  }
}

Derived()
    .foo();
);-]");
}

TEST_F(Super, no_superclass_bind)
{
  run(R";-](
class Base
{
  foo()
  {
    super.doesNotExist;  // Error at 'super': Can't use 'super' in a class with no superclass.
  }
}

Base()
    .foo();
);-]");
}

TEST_F(Super, no_superclass_call)
{
  run(R";-](
class Base
{
  foo()
  {
    super.doesNotExist(1);  // Error at 'super': Can't use 'super' in a class with no superclass.
  }
}

Base()
    .foo();
);-]");
}

TEST_F(Super, no_superclass_method)
{
  run(R";-](
class Base
{
}

    class Derived
    < Base
{
  foo()
  {
    super.doesNotExist(
        1);  // expect runtime error: Undefined property 'doesNotExist'.
  }
}

Derived()
    .foo();
);-]");
}

TEST_F(Super, parenthesized)
{
  run(R";-](
class A
{
  method() {}
}

    class B
    < A
{
  method()
  {
    // [line 8] Error at ')': Expect '.' after 'super'.
    (super).method();
  }
}
);-]");
}

TEST_F(Super, reassign_superclass)
{
  run(R";-](
class Base
{
  method()
  {
    print "Base.method()";
  }
}

    class Derived
    < Base
{
  method()
  {
    super.method();
  }
}

class OtherBase
{
  method()
  {
    print "OtherBase.method()";
  }
}

var derived = Derived();
derived.method();  // expect: Base.method()
Base = OtherBase;
derived.method();  // expect: Base.method()
);-]");
}

TEST_F(Super, super_at_top_level)
{
  run(R";-](
super.foo("bar");  // Error at 'super': Can't use 'super' outside of a class.
super.foo;  // Error at 'super': Can't use 'super' outside of a class.
);-]");
}

TEST_F(Super, super_in_closure_in_inherited_method)
{
  run(R";-](
class A
{
  say()
  {
    print "A";
  }
}

    class B
    < A
{
  getClosure()
  {
    fun closure()
    {
      super.say();
    }
    return closure;
  }

  say()
  {
    print "B";
  }
}

    class C
    < B
{
  say()
  {
    print "C";
  }
}

C()
    .getClosure()();  // expect: A
);-]");
}

TEST_F(Super, super_in_inherited_method)
{
  run(R";-](
class A
{
  say()
  {
    print "A";
  }
}

    class B
    < A
{
  test()
  {
    super.say();
  }

  say()
  {
    print "B";
  }
}

    class C
    < B
{
  say()
  {
    print "C";
  }
}

C()
    .test();  // expect: A
);-]");
}

TEST_F(Super, super_in_top_level_function)
{
  run(R";-](
super.bar();  // Error at 'super': Can't use 'super' outside of a class.
fun foo() {}
);-]");
}

TEST_F(Super, super_without_dot)
{
  run(R";-](
class A
{
}

    class B
    < A
{
  method()
  {
    // [line 6] Error at ';': Expect '.' after 'super'.
    super;
  }
}
);-]");
}

TEST_F(Super, super_without_name)
{
  run(R";-](
class A
{
}

    class B
    < A
{
  method()
  {
    super.;  // Error at ';': Expect superclass method name.
  }
}
);-]");
}

TEST_F(Super, this_in_superclass_method)
{
  run(R";-](
class Base
{
  init(a)
  {
    this.a = a;
  }
}

    class Derived
    < Base
{
  init(a, b)
  {
    super.init(a);
    this.b = b;
  }
}

var derived = Derived("a", "b");
print derived.a;  // expect: a
print derived.b;  // expect: b
);-]");
}
