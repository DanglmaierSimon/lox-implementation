#include <benchmark/benchmark.h>

#include "vm.h"

static void DoSetup(const benchmark::State&) {}

static void DoTeardown(const benchmark::State&) {}

static void BM_fibonacci(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
fun fib(n) {
  if (n < 2) return n;
  return fib(n - 2) + fib(n - 1);
}

print fib(35) == 9227465;
)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_instantiation(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
// This benchmark stresses instance creation and initializer calling.

class Foo {
  init() {}
}

var i = 0;
while (i < 500000) {
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  Foo();
  i = i + 1;
}
)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_instantiation_single(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
// This benchmark stresses instance creation and initializer calling.

class Foo {
  init() {}
}

Foo();
)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_tree(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
  class Tree {
  init(depth) {
    this.depth = depth;
    if (depth > 0) {
      this.a = Tree(depth - 1);
      this.b = Tree(depth - 1);
      this.c = Tree(depth - 1);
      this.d = Tree(depth - 1);
      this.e = Tree(depth - 1);
    }
  }

  walk() {
    if (this.depth == 0) return 0;
    return this.depth 
        + this.a.walk()
        + this.b.walk()
        + this.c.walk()
        + this.d.walk()
        + this.e.walk();
  }
}

var tree = Tree(8);
var start = clock();
for (var i = 0; i < 100; i = i + 1) {
  if (tree.walk() != 122068) print "Error";
}
print clock() - start;

)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_method_call(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
class Toggle {
  init(startState) {
    this.state = startState;
  }

  value() { return this.state; }

  activate() {
    this.state = !this.state;
    return this;
  }
}

class NthToggle < Toggle {
  init(startState, maxCounter) {
    super.init(startState);
    this.countMax = maxCounter;
    this.count = 0;
  }

  activate() {
    this.count = this.count + 1;
    if (this.count >= this.countMax) {
      super.activate();
      this.count = 0;
    }

    return this;
  }
}

var start = clock();
var n = 100000;
var val = true;
var toggle = Toggle(val);

for (var i = 0; i < n; i = i + 1) {
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
  val = toggle.activate().value();
}

print toggle.value();

val = true;
var ntoggle = NthToggle(val, 3);

for (var i = 0; i < n; i = i + 1) {
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
  val = ntoggle.activate().value();
}

print ntoggle.value();
print clock() - start;

)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_invocation(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
// This benchmark stresses just method invocation.

class Foo {
  method0() {}
  method1() {}
  method2() {}
  method3() {}
  method4() {}
  method5() {}
  method6() {}
  method7() {}
  method8() {}
  method9() {}
  method10() {}
  method11() {}
  method12() {}
  method13() {}
  method14() {}
  method15() {}
  method16() {}
  method17() {}
  method18() {}
  method19() {}
  method20() {}
  method21() {}
  method22() {}
  method23() {}
  method24() {}
  method25() {}
  method26() {}
  method27() {}
  method28() {}
  method29() {}
}

var foo = Foo();
var start = clock();
var i = 0;
while (i < 500000) {
  foo.method0();
  foo.method1();
  foo.method2();
  foo.method3();
  foo.method4();
  foo.method5();
  foo.method6();
  foo.method7();
  foo.method8();
  foo.method9();
  foo.method10();
  foo.method11();
  foo.method12();
  foo.method13();
  foo.method14();
  foo.method15();
  foo.method16();
  foo.method17();
  foo.method18();
  foo.method19();
  foo.method20();
  foo.method21();
  foo.method22();
  foo.method23();
  foo.method24();
  foo.method25();
  foo.method26();
  foo.method27();
  foo.method28();
  foo.method29();
  i = i + 1;
}

print clock() - start;

)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_equality(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"(
var i = 0;

var loopStart = clock();

while (i < 10000000) {
  i = i + 1;

  1; 1; 1; 2; 1; nil; 1; "str"; 1; true;
  nil; nil; nil; 1; nil; "str"; nil; true;
  true; true; true; 1; true; false; true; "str"; true; nil;
  "str"; "str"; "str"; "stru"; "str"; 1; "str"; nil; "str"; true;
}

var loopTime = clock() - loopStart;

var start = clock();

i = 0;
while (i < 10000000) {
  i = i + 1;

  1 == 1; 1 == 2; 1 == nil; 1 == "str"; 1 == true;
  nil == nil; nil == 1; nil == "str"; nil == true;
  true == true; true == 1; true == false; true == "str"; true == nil;
  "str" == "str"; "str" == "stru"; "str" == 1; "str" == nil; "str" == true;
}

var elapsed = clock() - start;
print "loop";
print loopTime;
print "elapsed";
print elapsed;
print "equals";
print elapsed - loopTime;
)";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

static void BM_compile_and_run_empty_file(benchmark::State& state)
{
  VM vm;

  constexpr auto source = R"()";

  for (auto _ : state) {
    vm.interpret(source);
  }
}

BENCHMARK(BM_fibonacci);
BENCHMARK(BM_instantiation);
BENCHMARK(BM_instantiation_single);
BENCHMARK(BM_tree);
BENCHMARK(BM_invocation);
BENCHMARK(BM_method_call);
BENCHMARK(BM_equality);
BENCHMARK(BM_compile_and_run_empty_file);

// Run the benchmark
BENCHMARK_MAIN();