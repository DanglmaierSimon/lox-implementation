#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Variable : public End2EndTest
{
};

TEST_F(Variable, collide_with_parameter)
{
  run(R";-](
fun foo(a)
{
  var a;  // Error at 'a': Already a variable with this name in this scope.
}
);-]");
}

TEST_F(Variable, duplicate_local)
{
  run(R";-](
{
  var a = "value";
  var a = "other";  // Error at 'a': Already a variable with this name in this scope.
}
);-]");
}

TEST_F(Variable, duplicate_parameter)
{
  run(R";-](
fun foo(arg, arg)
{  // Error at 'arg': Already a variable with this name in this scope.
  "body";
}
);-]");
}

TEST_F(Variable, early_bound)
{
  run(R";-](
var a = "outer";
{
  fun foo()
  {
    print a;
  }

  foo();  // expect: outer
  var a = "inner";
  foo();  // expect: outer
}
);-]");
}

TEST_F(Variable, in_middle_of_block)
{
  run(R";-](
{
  var a = "a";
  print a;  // expect: a
  var b = a + " b";
  print b;  // expect: a b
  var c = a + " c";
  print c;  // expect: a c
  var d = b + " d";
  print d;  // expect: a b d
}
);-]");
}

TEST_F(Variable, in_nested_block)
{
  run(R";-](
{
  var a = "outer";
  {
    print a;  // expect: outer
  }
}
);-]");
}

TEST_F(Variable, local_from_method)
{
  run(R";-](
var foo = "variable";

class Foo
{
  method()
  {
    print foo;
  }
}

Foo()
    .method();  // expect: variable
);-]");
}

TEST_F(Variable, redeclare_global)
{
  run(R";-](
var a = "1";
var a;
print a;  // expect: nil
);-]");
}

TEST_F(Variable, redefine_global)
{
  run(R";-](
var a = "1";
var a = "2";
print a;  // expect: 2
);-]");
}

TEST_F(Variable, scope_reuse_in_different_blocks)
{
  run(R";-](
{
  var a = "first";
  print a;  // expect: first
}

{
  var a = "second";
  print a;  // expect: second
}
);-]");
}

TEST_F(Variable, shadow_and_local)
{
  run(R";-](
{
  var a = "outer";
  {
    print a;  // expect: outer
    var a = "inner";
    print a;  // expect: inner
  }
}
);-]");
}

TEST_F(Variable, shadow_global)
{
  run(R";-](
var a = "global";
{
  var a = "shadow";
  print a;  // expect: shadow
}
print a;  // expect: global
);-]");
}

TEST_F(Variable, shadow_local)
{
  run(R";-](
{
  var a = "local";
  {
    var a = "shadow";
    print a;  // expect: shadow
  }
  print a;  // expect: local
}
);-]");
}

TEST_F(Variable, undefined_global)
{
  run(R";-](
print notDefined;  // expect runtime error: Undefined variable 'notDefined'.
);-]");
}

TEST_F(Variable, undefined_local)
{
  run(R";-](
{
  print notDefined;  // expect runtime error: Undefined variable 'notDefined'.
}
);-]");
}

TEST_F(Variable, uninitialized)
{
  run(R";-](
var a;
print a;  // expect: nil
);-]");
}

TEST_F(Variable, unreached_undefined)
{
  run(R";-](
if (false) {
  print notDefined;
}

print "ok";  // expect: ok
);-]");
}

TEST_F(Variable, use_false_as_var)
{
  run(R";-](
// [line 2] Error at 'false': Expect variable name.
var false = "value";
);-]");
}

TEST_F(Variable, use_global_in_initializer)
{
  run(R";-](
var a = "value";
var a = a;
print a;  // expect: value
);-]");
}

TEST_F(Variable, use_local_in_initializer)
{
  run(R";-](
var a = "outer";
{
  var a = a;  // Error at 'a': Can't read local variable in its own initializer.
}
);-]");
}

TEST_F(Variable, use_nil_as_var)
{
  run(R";-](
// [line 2] Error at 'nil': Expect variable name.
var nil = "value";
);-]");
}

TEST_F(Variable, use_this_as_var)
{
  run(R";-](
// [line 2] Error at 'this': Expect variable name.
var this = "value";
);-]");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}