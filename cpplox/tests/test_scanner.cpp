#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"
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

TEST(ScannerTest, CommentsAreIgnores)
{
  Scanner s {R"(
    123 // 324
    var // if // while
    if / ///
  )"};

  const auto tokens = scanAll(s);

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

TEST(ScannerTest, CheckSingleCharacterTokens)
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

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}