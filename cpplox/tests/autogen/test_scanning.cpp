#include <gtest/gtest.h>

#include "testhelper.h"

class Scanning : public End2EndTest
{
};

TEST_F(Scanning, identifiers)
{
  run(R";-](
andy formless fo _ _123 _abc ab123
    abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_

    // expect: IDENTIFIER andy null
    // expect: IDENTIFIER formless null
    // expect: IDENTIFIER fo null
    // expect: IDENTIFIER _ null
    // expect: IDENTIFIER _123 null
    // expect: IDENTIFIER _abc null
    // expect: IDENTIFIER ab123 null
    // expect: IDENTIFIER
    // abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_ null
    // expect: EOF  null
);-]");
}

TEST_F(Scanning, keywords)
{
  run(R";-](
and class else false for fun if nil or return super this true var while

// expect: AND and null
// expect: CLASS class null
// expect: ELSE else null
// expect: FALSE false null
// expect: FOR for null
// expect: FUN fun null
// expect: IF if null
// expect: NIL nil null
// expect: OR or null
// expect: RETURN return null
// expect: SUPER super null
// expect: THIS this null
// expect: TRUE true null
// expect: VAR var null
// expect: WHILE while null
// expect: EOF  null
);-]");
}

TEST_F(Scanning, numbers)
{
  run(R";-](
123 123.456 .456 123.

    // expect: NUMBER 123 123.0
    // expect: NUMBER 123.456 123.456
    // expect: DOT . null
    // expect: NUMBER 456 456.0
    // expect: NUMBER 123 123.0
    // expect: DOT . null
    // expect: EOF  null
);-]");
}

TEST_F(Scanning, punctuators)
{
  run(R";-](
() {};
, +-* != == <=> = != <> /
                             .

                             // expect: LEFT_PAREN ( null
                             // expect: RIGHT_PAREN ) null
                             // expect: LEFT_BRACE { null
                             // expect: RIGHT_BRACE } null
                             // expect: SEMICOLON ; null
                             // expect: COMMA , null
                             // expect: PLUS + null
                             // expect: MINUS - null
                             // expect: STAR * null
                             // expect: BANG_EQUAL != null
                             // expect: EQUAL_EQUAL == null
                             // expect: LESS_EQUAL <= null
                             // expect: GREATER_EQUAL >= null
                             // expect: BANG_EQUAL != null
                             // expect: LESS < null
                             // expect: GREATER > null
                             // expect: SLASH / null
                             // expect: DOT . null
                             // expect: EOF  null
);-]");
}

TEST_F(Scanning, strings)
{
  run(R";-](
""
    "string"

    // expect: STRING ""
    // expect: STRING "string" string
    // expect: EOF  null
);-]");
}

TEST_F(Scanning, whitespace)
{
  run(R";-](
space tabs newlines

    end

    // expect: IDENTIFIER space null
    // expect: IDENTIFIER tabs null
    // expect: IDENTIFIER newlines null
    // expect: IDENTIFIER end null
    // expect: EOF  null
);-]");
}
