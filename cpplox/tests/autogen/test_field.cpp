#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Field : public End2EndTest
{
};

TEST_F(Field, call_function_field)
{
  run(R";-](
class Foo
{
}

fun bar(a, b)
{
  print "bar";
  print a;
  print b;
}

var foo = Foo();
foo.bar = bar;

foo.bar(1, 2);
// expect: bar
// expect: 1
// expect: 2
);-]");
}

TEST_F(Field, call_nonfunction_field)
{
  run(R";-](
class Foo
{
}

var foo = Foo();
foo.bar = "not fn";

foo.bar();  // expect runtime error: Can only call functions and classes.
);-]");
}

TEST_F(Field, get_and_set_method)
{
  run(R";-](
// Bound methods have identity equality.
class Foo
{
  method(a)
  {
    print "method";
    print a;
  }
  other(a)
  {
    print "other";
    print a;
  }
}

var foo = Foo();
var method = foo.method;

// Setting a property shadows the instance method.
foo.method = foo.other;
foo.method(1);
// expect: other
// expect: 1

// The old method handle still points to the original method.
method(2);
// expect: method
// expect: 2
);-]");
}

TEST_F(Field, get_on_bool)
{
  run(R";-](
true.foo;  // expect runtime error: Only instances have properties.
);-]");
}

TEST_F(Field, get_on_class)
{
  run(R";-](
class Foo
{
} Foo.bar;  // expect runtime error: Only instances have properties.
);-]");
}

TEST_F(Field, get_on_function)
{
  run(R";-](
fun foo() {}

foo.bar;  // expect runtime error: Only instances have properties.
);-]");
}

TEST_F(Field, get_on_nil)
{
  run(R";-](
nil.foo;  // expect runtime error: Only instances have properties.
);-]");
}

TEST_F(Field, get_on_num)
{
  run(R";-](
123.foo;  // expect runtime error: Only instances have properties.
);-]");
}

TEST_F(Field, get_on_string)
{
  run(R";-](
"str".foo;  // expect runtime error: Only instances have properties.
);-]");
}

TEST_F(Field, many)
{
  run(R";-](
class Foo
{
}

var foo = Foo();
fun setFields()
{
  foo.bilberry = "bilberry";
  foo.lime = "lime";
  foo.elderberry = "elderberry";
  foo.raspberry = "raspberry";
  foo.gooseberry = "gooseberry";
  foo.longan = "longan";
  foo.mandarine = "mandarine";
  foo.kiwifruit = "kiwifruit";
  foo.orange = "orange";
  foo.pomegranate = "pomegranate";
  foo.tomato = "tomato";
  foo.banana = "banana";
  foo.juniper = "juniper";
  foo.damson = "damson";
  foo.blackcurrant = "blackcurrant";
  foo.peach = "peach";
  foo.grape = "grape";
  foo.mango = "mango";
  foo.redcurrant = "redcurrant";
  foo.watermelon = "watermelon";
  foo.plumcot = "plumcot";
  foo.papaya = "papaya";
  foo.cloudberry = "cloudberry";
  foo.rambutan = "rambutan";
  foo.salak = "salak";
  foo.physalis = "physalis";
  foo.huckleberry = "huckleberry";
  foo.coconut = "coconut";
  foo.date = "date";
  foo.tamarind = "tamarind";
  foo.lychee = "lychee";
  foo.raisin = "raisin";
  foo.apple = "apple";
  foo.avocado = "avocado";
  foo.nectarine = "nectarine";
  foo.pomelo = "pomelo";
  foo.melon = "melon";
  foo.currant = "currant";
  foo.plum = "plum";
  foo.persimmon = "persimmon";
  foo.olive = "olive";
  foo.cranberry = "cranberry";
  foo.boysenberry = "boysenberry";
  foo.blackberry = "blackberry";
  foo.passionfruit = "passionfruit";
  foo.mulberry = "mulberry";
  foo.marionberry = "marionberry";
  foo.plantain = "plantain";
  foo.lemon = "lemon";
  foo.yuzu = "yuzu";
  foo.loquat = "loquat";
  foo.kumquat = "kumquat";
  foo.salmonberry = "salmonberry";
  foo.tangerine = "tangerine";
  foo.durian = "durian";
  foo.pear = "pear";
  foo.cantaloupe = "cantaloupe";
  foo.quince = "quince";
  foo.guava = "guava";
  foo.strawberry = "strawberry";
  foo.nance = "nance";
  foo.apricot = "apricot";
  foo.jambul = "jambul";
  foo.grapefruit = "grapefruit";
  foo.clementine = "clementine";
  foo.jujube = "jujube";
  foo.cherry = "cherry";
  foo.feijoa = "feijoa";
  foo.jackfruit = "jackfruit";
  foo.fig = "fig";
  foo.cherimoya = "cherimoya";
  foo.pineapple = "pineapple";
  foo.blueberry = "blueberry";
  foo.jabuticaba = "jabuticaba";
  foo.miracle = "miracle";
  foo.dragonfruit = "dragonfruit";
  foo.satsuma = "satsuma";
  foo.tamarillo = "tamarillo";
  foo.honeydew = "honeydew";
}

setFields();

fun printFields()
{
  print foo.apple;  // expect: apple
  print foo.apricot;  // expect: apricot
  print foo.avocado;  // expect: avocado
  print foo.banana;  // expect: banana
  print foo.bilberry;  // expect: bilberry
  print foo.blackberry;  // expect: blackberry
  print foo.blackcurrant;  // expect: blackcurrant
  print foo.blueberry;  // expect: blueberry
  print foo.boysenberry;  // expect: boysenberry
  print foo.cantaloupe;  // expect: cantaloupe
  print foo.cherimoya;  // expect: cherimoya
  print foo.cherry;  // expect: cherry
  print foo.clementine;  // expect: clementine
  print foo.cloudberry;  // expect: cloudberry
  print foo.coconut;  // expect: coconut
  print foo.cranberry;  // expect: cranberry
  print foo.currant;  // expect: currant
  print foo.damson;  // expect: damson
  print foo.date;  // expect: date
  print foo.dragonfruit;  // expect: dragonfruit
  print foo.durian;  // expect: durian
  print foo.elderberry;  // expect: elderberry
  print foo.feijoa;  // expect: feijoa
  print foo.fig;  // expect: fig
  print foo.gooseberry;  // expect: gooseberry
  print foo.grape;  // expect: grape
  print foo.grapefruit;  // expect: grapefruit
  print foo.guava;  // expect: guava
  print foo.honeydew;  // expect: honeydew
  print foo.huckleberry;  // expect: huckleberry
  print foo.jabuticaba;  // expect: jabuticaba
  print foo.jackfruit;  // expect: jackfruit
  print foo.jambul;  // expect: jambul
  print foo.jujube;  // expect: jujube
  print foo.juniper;  // expect: juniper
  print foo.kiwifruit;  // expect: kiwifruit
  print foo.kumquat;  // expect: kumquat
  print foo.lemon;  // expect: lemon
  print foo.lime;  // expect: lime
  print foo.longan;  // expect: longan
  print foo.loquat;  // expect: loquat
  print foo.lychee;  // expect: lychee
  print foo.mandarine;  // expect: mandarine
  print foo.mango;  // expect: mango
  print foo.marionberry;  // expect: marionberry
  print foo.melon;  // expect: melon
  print foo.miracle;  // expect: miracle
  print foo.mulberry;  // expect: mulberry
  print foo.nance;  // expect: nance
  print foo.nectarine;  // expect: nectarine
  print foo.olive;  // expect: olive
  print foo.orange;  // expect: orange
  print foo.papaya;  // expect: papaya
  print foo.passionfruit;  // expect: passionfruit
  print foo.peach;  // expect: peach
  print foo.pear;  // expect: pear
  print foo.persimmon;  // expect: persimmon
  print foo.physalis;  // expect: physalis
  print foo.pineapple;  // expect: pineapple
  print foo.plantain;  // expect: plantain
  print foo.plum;  // expect: plum
  print foo.plumcot;  // expect: plumcot
  print foo.pomegranate;  // expect: pomegranate
  print foo.pomelo;  // expect: pomelo
  print foo.quince;  // expect: quince
  print foo.raisin;  // expect: raisin
  print foo.rambutan;  // expect: rambutan
  print foo.raspberry;  // expect: raspberry
  print foo.redcurrant;  // expect: redcurrant
  print foo.salak;  // expect: salak
  print foo.salmonberry;  // expect: salmonberry
  print foo.satsuma;  // expect: satsuma
  print foo.strawberry;  // expect: strawberry
  print foo.tamarillo;  // expect: tamarillo
  print foo.tamarind;  // expect: tamarind
  print foo.tangerine;  // expect: tangerine
  print foo.tomato;  // expect: tomato
  print foo.watermelon;  // expect: watermelon
  print foo.yuzu;  // expect: yuzu
}

printFields();
);-]");
}

TEST_F(Field, method_binds_this)
{
  run(R";-](
class Foo
{
  sayName(a)
  {
    print this.name;
    print a;
  }
}

var foo1 = Foo();
foo1.name = "foo1";

var foo2 = Foo();
foo2.name = "foo2";

// Store the method reference on another object.
foo2.fn = foo1.sayName;
// Still retains original receiver.
foo2.fn(1);
// expect: foo1
// expect: 1
);-]");
}

TEST_F(Field, method)
{
  run(R";-](
class Foo
{
  bar(arg)
  {
    print arg;
  }
}

var bar = Foo().bar;
print "got method";  // expect: got method
bar("arg");  // expect: arg
);-]");
}

TEST_F(Field, on_instance)
{
  run(R";-](
class Foo
{
}

var foo = Foo();

print foo.bar = "bar value";  // expect: bar value
print foo.baz = "baz value";  // expect: baz value

print foo.bar;  // expect: bar value
print foo.baz;  // expect: baz value
);-]");
}

TEST_F(Field, set_evaluation_order)
{
  run(R";-](
undefined1.bar  // expect runtime error: Undefined variable 'undefined1'.
    = undefined2;
);-]");
}

TEST_F(Field, set_on_bool)
{
  run(R";-](
true.foo = "value";  // expect runtime error: Only instances have fields.
);-]");
}

TEST_F(Field, set_on_class)
{
  run(R";-](
class Foo
{
} Foo.bar = "value";  // expect runtime error: Only instances have fields.
);-]");
}

TEST_F(Field, set_on_function)
{
  run(R";-](
fun foo() {}

foo.bar = "value";  // expect runtime error: Only instances have fields.
);-]");
}

TEST_F(Field, set_on_nil)
{
  run(R";-](
nil.foo = "value";  // expect runtime error: Only instances have fields.
);-]");
}

TEST_F(Field, set_on_num)
{
  run(R";-](
123.foo = "value";  // expect runtime error: Only instances have fields.
);-]");
}

TEST_F(Field, set_on_string)
{
  run(R";-](
"str".foo = "value";  // expect runtime error: Only instances have fields.
);-]");
}

TEST_F(Field, undefined)
{
  run(R";-](
class Foo
{
} var foo = Foo();

foo.bar;  // expect runtime error: Undefined property 'bar'.
);-]");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}