#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "lox/scanner.h"

namespace
{

std::vector<Token> scanAll(Scanner s)
{
  std::vector<Token> retval;
  while (true) {
    auto t = s.scanToken();
    retval.push_back(t);

    if (retval.back().type() == TokenType::END_OF_FILE) {
      return retval;
    }
  }
}

std::vector<Token> scanAll(std::string_view s)
{
  Scanner scanner {s.data()};

  return scanAll(scanner);
}

void printTokens(std::vector<Token> tokens)
{
  std::for_each(tokens.cbegin(), tokens.cend(), [](Token t) {
    std::cout << t << std::endl;
  });
}

}  // namespace

TEST(ScannerTest, EmptySourceFileEmitsEOF)
{
  Scanner s {""};

  ASSERT_EQ(s.scanToken().type(), TokenType::END_OF_FILE);
}

TEST(ScannerTest, EmptySourceLeadsToOnly1EOFToken)
{
  Scanner s {""};

  ASSERT_EQ(scanAll(s).size(), 1);
}

TEST(ScannerTest, EmptySourceLinesStartsAt1)
{
  Scanner s {""};

  ASSERT_EQ(s.scanToken().line(), 1);
}

TEST(ScannerTest, WhitespaceIgnored)
{
  Scanner s {"       \t\t\t\t\r\r\r\r       \n\n\n"};

  ASSERT_EQ(scanAll(s).size(), 1);
}

TEST(ScannerTest, NewlineUpdatesLineNumber)
{
  Scanner s1 {"\n\n\n"};

  auto t = s1.scanToken();
  ASSERT_EQ(t.line(), 4);  // line gets incremented 3 times
}

TEST(ScannerTest, TestInteger)
{
  std::random_device rd;  // obtain a random number from hardware
  std::mt19937 gen(rd());  // seed the generator
  std::uniform_int_distribution<> distr(
      0, std::numeric_limits<int>::max());  // define the range

  for (int i = 0; i < 1000000; i++) {
    const auto num = distr(gen);
    const auto str = std::to_string(num);

    Scanner s {str.c_str()};
    const auto t = s.scanToken();
    ASSERT_EQ(t.type(), TokenType::NUMBER);
    ASSERT_EQ(t.string(), std::to_string(num));
  }
}

TEST(ScannerTest, CommentsAreIgnored)
{
  Scanner s {R"(
    123 // 324
    var // if // while
    if / ///
  )"};

  const auto tokens = scanAll(s);
  printTokens(tokens);

  ASSERT_EQ(tokens.size(), 5);
  ASSERT_EQ(tokens.back().type(), TokenType::END_OF_FILE);

  ASSERT_EQ(tokens.at(0).type(), TokenType::NUMBER);
  ASSERT_EQ(tokens.at(0).string(), "123");

  ASSERT_EQ(tokens.at(1).type(), TokenType::VAR);
  ASSERT_EQ(tokens.at(1).string(), "var");

  ASSERT_EQ(tokens.at(2).type(), TokenType::IF);
  ASSERT_EQ(tokens.at(2).string(), "if");

  ASSERT_EQ(tokens.at(3).type(), TokenType::SLASH);
  ASSERT_EQ(tokens.at(3).string(), "/");
}

// Keyword tests

TEST(ScannerTest, CheckKeyword_and)
{
  Scanner s {"and"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::AND);
  ASSERT_EQ(t.string(), "and");
}

TEST(ScannerTest, CheckKeyword_class)
{
  Scanner s {"class"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::CLASS);
  ASSERT_EQ(t.string(), "class");
}

TEST(ScannerTest, CheckKeyword_else)
{
  Scanner s {"else"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::ELSE);
  ASSERT_EQ(t.string(), "else");
}

TEST(ScannerTest, CheckKeyword_false)
{
  Scanner s {"false"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::FALSE);
  ASSERT_EQ(t.string(), "false");
}

TEST(ScannerTest, CheckKeyword_for)
{
  Scanner s {"for"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::FOR);
  ASSERT_EQ(t.string(), "for");
}

TEST(ScannerTest, CheckKeyword_fun)
{
  Scanner s {"fun"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::FUN);
  ASSERT_EQ(t.string(), "fun");
}

TEST(ScannerTest, CheckKeyword_if)
{
  Scanner s {"if"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::IF);
  ASSERT_EQ(t.string(), "if");
}

TEST(ScannerTest, CheckKeyword_nil)
{
  Scanner s {"nil"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::NIL);
  ASSERT_EQ(t.string(), "nil");
}

TEST(ScannerTest, CheckKeyword_or)
{
  Scanner s {"or"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::OR);
  ASSERT_EQ(t.string(), "or");
}

TEST(ScannerTest, CheckKeyword_print)
{
  Scanner s {"print"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::PRINT);
  ASSERT_EQ(t.string(), "print");
}

TEST(ScannerTest, CheckKeyword_return)
{
  Scanner s {"return"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::RETURN);
  ASSERT_EQ(t.string(), "return");
}

TEST(ScannerTest, CheckKeyword_super)
{
  Scanner s {"super"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::SUPER);
  ASSERT_EQ(t.string(), "super");
}

TEST(ScannerTest, CheckKeyword_this)
{
  Scanner s {"this"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::THIS);
  ASSERT_EQ(t.string(), "this");
}

TEST(ScannerTest, CheckKeyword_true)
{
  Scanner s {"true"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::TRUE);
  ASSERT_EQ(t.string(), "true");
}

TEST(ScannerTest, CheckKeyword_var)
{
  Scanner s {"var"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::VAR);
  ASSERT_EQ(t.string(), "var");
}

TEST(ScannerTest, CheckKeyword_while)
{
  Scanner s {"while"};
  const auto t = s.scanToken();
  ASSERT_EQ(t.type(), TokenType::WHILE);
  ASSERT_EQ(t.string(), "while");
}

TEST(ScannerTest, CheckeCharacterTokens)
{
  Scanner s {R"(
    (
    )
    {
    }
    ;
    ,
    .
    -
    +
    /
    *
    !=
    !
    ==
    =
    <=
    <
    >=
    >
  )"};

  const auto actual = scanAll(s);
  const auto expected = std::vector {
      TokenType::LEFT_PAREN,  TokenType::RIGHT_PAREN, TokenType::LEFT_BRACE,
      TokenType::RIGHT_BRACE, TokenType::SEMICOLON,   TokenType::COMMA,
      TokenType::DOT,         TokenType::MINUS,       TokenType::PLUS,
      TokenType::SLASH,       TokenType::STAR,        TokenType::BANG_EQUAL,
      TokenType::BANG,        TokenType::EQUAL_EQUAL, TokenType::EQUAL,
      TokenType::LESS_EQUAL,  TokenType::LESS,        TokenType::GREATER_EQUAL,
      TokenType::GREATER,     TokenType::END_OF_FILE};

  ASSERT_EQ(actual.size(), expected.size());

  for (size_t i = 0; i < actual.size(); i++) {
    std::cout << "index: " << i << std::endl;
    EXPECT_EQ(actual.at(i).type(), expected.at(i));
  }
}

TEST(ScannerTest, Identifiers)
{
  Scanner s {R"(
    andy formless fo _ _123 _abc ab123
    abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_
  )"};

  auto tokens = scanAll(s);

  const auto expectedTokenType = TokenType::IDENTIFIER;

  std::vector<std::string_view> expected = {
      "andy",
      "formless",
      "fo",
      "_",
      "_123",
      "_abc",
      "ab123",
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"};

  ASSERT_EQ(expected.size() + 1, tokens.size());
  ASSERT_EQ(tokens.back().type(), TokenType::END_OF_FILE);

  tokens.pop_back();

  ASSERT_EQ(expected.size(), tokens.size());

  for (size_t i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens.at(i).type(), expectedTokenType);
    EXPECT_EQ(tokens.at(i).string(), expected.at(i));
  }
}

TEST(ScannerTest, Numbers)
{
  auto tokens = scanAll("123 123.456 .456 123.");

  ASSERT_EQ(tokens.back().type(), TokenType::END_OF_FILE);

  tokens.pop_back();

  struct exp
  {
    TokenType type;
    std::string literal;
  };

  std::vector<exp> expected {{TokenType::NUMBER, "123"},
                             {TokenType::NUMBER, "123.456"},
                             {TokenType::DOT, "."},
                             {TokenType::NUMBER, "456"},
                             {TokenType::NUMBER, "123"},
                             {TokenType::DOT, "."}};

  ASSERT_EQ(expected.size(), tokens.size());

  for (size_t i = 0; i < tokens.size(); i++) {
    const auto t = tokens.at(i);
    const auto e = expected.at(i);

    EXPECT_EQ(t.type(), e.type);
    EXPECT_EQ(t.string(), e.literal);
  }
}

TEST(ScannerTest, Whitespace)
{
  auto tokens = scanAll(
      R"(
      space tabs newlines

    end

    )");

  ASSERT_EQ(tokens.back().type(), TokenType::END_OF_FILE);

  tokens.pop_back();

  ASSERT_EQ(tokens.size(), 4);

  auto t1 = tokens.at(0);
  auto t2 = tokens.at(1);
  auto t3 = tokens.at(2);
  auto t4 = tokens.at(3);

  EXPECT_EQ(t1.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(t1.string(), "space");

  EXPECT_EQ(t2.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(t2.string(), "tabs");

  EXPECT_EQ(t3.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(t3.string(), "newlines");

  EXPECT_EQ(t4.type(), TokenType::IDENTIFIER);
  EXPECT_EQ(t4.string(), "end");
}

TEST(ScannerTest, Strings)
{
  auto tokens = scanAll(
      R"(
      ""
    "string"
    "this
    is
    a
    multiline
    string"
    )");

  ASSERT_GT(tokens.size(), 3);

  ASSERT_EQ(tokens.at(0).type(), TokenType::STRING);
  ASSERT_EQ(tokens.at(0).string(), "\"\"");

  ASSERT_EQ(tokens.at(1).type(), TokenType::STRING);
  ASSERT_EQ(tokens.at(1).string(), "\"string\"");

  ASSERT_EQ(tokens.at(2).type(), TokenType::STRING);
  ASSERT_EQ(tokens.at(2).string(),
            R"("this
    is
    a
    multiline
    string")");
}

TEST(ScannerTest, UnexpectedCharacter)
{
  auto tokens = scanAll("a | b");

  ASSERT_EQ(tokens.size(), 4);

  ASSERT_EQ(tokens.at(1).type(), TokenType::ERROR);
}

TEST(ScannerTest, BlockCommentSimple)
{
  {
    auto tokens = scanAll("/**/");

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens.at(0).type(), TokenType::END_OF_FILE);
  }

  {
    // multiline comments get terminated by the first multiline-comment
    // terminator */ and do not nest
    auto tokens = scanAll("/* /* */ */");

    printTokens(tokens);
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens.at(0).type(), TokenType::STAR);
    EXPECT_EQ(tokens.at(1).type(), TokenType::SLASH);
  }
}

TEST(ScannerTest, BlockCommentsIgnoreStuffInside)
{
  auto strings = {
      "/*   a  */",  // identifier in comment
      "/* var a = 43; */",  // variable declaration in comment
      "/*print 5;*/",  // statement in comment
      "/* // */",  // single line comment in block comment
      "/*  / *  * / */",  // end-of-blockcomment chars but with spaces inbetween
      "/* /* /* /* /* /* */",  // multiple block comments dont nest
      R"(/*
  
  




   */)",  // multiple lines in blockcomment
      R"(/*
    var 1 = 12;
    // 
    if while
    salfhsdlkhfjdklsajhfdlksjhkl
    */)",  // random stuff in multiline comment
      "/* \"this is a string \" */",  // string in comment
      "/* \" this is an unterminated string */",  // unterminated string in
                                                  // comment
      "// /*",  // single line comment does not cause multiline comment to
                // start,
      "/*//*/"  // this is just 1 multiline comment
  };

  for (auto str : strings) {
    auto tokens = scanAll(str);

    std::cout << "Input:" << str << std::endl;
    std::cout << "Tokens:" << std::endl;

    for (auto t : tokens) {
      std::cout << t << std::endl;
    }

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens.at(0).type(),
              TokenType::END_OF_FILE);  // verify that simple comments lead to
                                        // no tokens
  }
}

TEST(ScannerTest, UnterminatedMultilineComment)
{
  auto tokens = scanAll("/*");
  ASSERT_EQ(tokens.size(), 2);
  ASSERT_EQ(tokens.at(0).type(), TokenType::ERROR);
  ASSERT_EQ(tokens.at(0).string(), "Unterminated multiline comment.");
}

TEST(ScannerTest,
     DearGodWhyDidIDecideToAddMultilineCommentsThereAreSoManyEdgeCases)
{
  // collection of edge cases

  {  // multiline comments generally act the same way as a space, so they should
    // break multi-character tokens

    auto tokens = scanAll("!= !/**/=");  // the first != should be parsed as
                                         // expected, the second one not

    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens.at(0).type(), TokenType::BANG_EQUAL);
    EXPECT_EQ(tokens.at(1).type(), TokenType::BANG);
    EXPECT_EQ(tokens.at(2).type(), TokenType::EQUAL);
  }

  {  // comments are right-associative, i guess
    // line 1 should be parsed as 1 single line comment
    // line 2 is 1 multiline comment with a slash after it
    auto tokens = scanAll(R"(
      ///*
      /**//
    )");

    ASSERT_EQ(tokens.size(), 2);  // slash and end of file tokens
    EXPECT_EQ(tokens.at(0).type(), TokenType::SLASH);
  }

  {
    // dont parse /*/ as a valid mutliline comment start and end
    auto tokens = scanAll("/*/");

    printTokens(tokens);
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens.at(0).type(), TokenType::ERROR);
  }

  {  // multiline comments get terminated by multiline comment markers within
    // strings
    auto tokens = scanAll("/* var a = \"*/\"\"");

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens.at(0).type(), TokenType::STRING);
  }
}

TEST(ScannerTest, MultilineCommentsActAsSpaces)
{
  // Take a few sample lox programs, scan them, then replace all spaces with
  // multiline comments, scan them again, the end results should be equal

  std::vector<std::string> programs = {
      R";-](
var f;
var g;

{
  var local = "local";
  fun f_()
  {
    print local;
    local = "after f";
    print local;
  }
  f = f_;

  fun g_()
  {
    print local;
    local = "after g";
    print local;
  }
  g = g_;
}

f();
// expect: local
// expect: after f

g();
// expect: after f
// expect: after g
);-]",
      R";-](
print 123;  // expect: 123
print 987654;  // expect: 987654
print 0;  // expect: 0
print - 0;  // expect: -0

print 123.456;  // expect: 123.456
print - 0.001;  // expect: -0.001
);-]",
      R";-](
var f1;
var f2;
var f3;

for (var i = 1; i < 4; i = i + 1) {
  var j = i;
  fun f()
  {
    print i;
    print j;
  }

  if (j == 1)
    f1 = f;
  else if (j == 2)
    f2 = f;
  else
    f3 = f;
}

f1();  // expect: 4
       // expect: 1
f2();  // expect: 4
       // expect: 2
f3();  // expect: 4
       // expect: 3
);-]",
      R";-](
print true == true;  // expect: true
print true == false;  // expect: false
print false == true;  // expect: false
print false == false;  // expect: true

// Not equal to other types.
print true == 1;  // expect: false
print false == 0;  // expect: false
print true == "true";  // expect: false
print false == "false";  // expect: false
print false == "";  // expect: false

print true != true;  // expect: false
print true != false;  // expect: true
print false != true;  // expect: true
print false != false;  // expect: false

// Not equal to other types.
print true != 1;  // expect: true
print false != 0;  // expect: true
print true != "true";  // expect: true
print false != "false";  // expect: true
print false != "";  // expect: true
);-]"};

  for (auto program : programs) {
    auto replaced = std::regex_replace(program, std::regex(" "), "/* */");
    auto after = scanAll(replaced);
    auto before = scanAll(program);

    std::cout << "=========================================" << std::endl;
    std::cout << "Program before: " << program << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Program after: " << replaced << std::endl;
    std::cout << "=========================================" << std::endl;

    ASSERT_EQ(after.size(), before.size());

    for (size_t i = 0; i < after.size(); i++) {
      std::cout << "Before: " << before.at(i) << std::endl;
      std::cout << "After: " << after.at(i) << std::endl;
      EXPECT_EQ(after.at(i).type(), before.at(i).type());
    }
  }
}

// TEST(ScannerTest, ListCreation)
// {
//   auto tokens = scanAll("a = list(12, 34)");

//   ASSERT_EQ(tokens.size(), 9);

//   EXPECT_EQ(tokens.at(2).type(), TokenType::LIST);
//   EXPECT_EQ(tokens.at(3).type(), TokenType::LEFT_PAREN);

//   EXPECT_EQ(tokens.at(4).type(), TokenType::NUMBER);
//   EXPECT_EQ(tokens.at(4).string(), "12");

//   EXPECT_EQ(tokens.at(6).type(), TokenType::NUMBER);
//   EXPECT_EQ(tokens.at(6).string(), "34");

//   EXPECT_EQ(tokens.at(7).type(), TokenType::RIGHT_PAREN);
// }

// TEST(ScannerTest, ListIndexing)
// {
//   auto tokens = scanAll("var b = a[12]");

//   ASSERT_EQ(tokens.size(), 8);

//   EXPECT_EQ(tokens.at(3).type(), TokenType::IDENTIFIER);
//   EXPECT_EQ(tokens.at(3).string(), "a");

//   EXPECT_EQ(tokens.at(4).type(), TokenType::LEFT_BRACKET);

//   EXPECT_EQ(tokens.at(5).type(), TokenType::NUMBER);
//   EXPECT_EQ(tokens.at(5).string(), "12");

//   EXPECT_EQ(tokens.at(6).type(), TokenType::RIGHT_BRACKET);
// }