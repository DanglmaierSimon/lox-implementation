#pragma once

#include <iostream>

enum class TokenType
{
  // Single-character tokens.
  LEFT_PAREN,  // (
  RIGHT_PAREN,  // )
  LEFT_BRACE,  // {
  RIGHT_BRACE,  // }
  /* LEFT_BRACKET,  // [
   RIGHT_BRACKET,  // ] */
  COMMA,  // ,
  DOT,  // .
  MINUS,  // -
  PLUS,  // +
  SEMICOLON,  // ;
  SLASH,  // /
  STAR,  // *

  // One or two character tokens.
  BANG,  // !
  BANG_EQUAL,  // !=
  EQUAL,  // =
  EQUAL_EQUAL,  // ==
  GREATER,  // >
  GREATER_EQUAL,  // >=
  LESS,  // <
  LESS_EQUAL,  // <=

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FOR,
  FUN,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  CONST,
  WHILE,

  // Special tokens
  ERROR,
  END_OF_FILE,
};

std::ostream& operator<<(std::ostream& os, TokenType t);

class Token
{
public:
  constexpr Token()
      : Token {TokenType::ERROR, "", 0}
  {
  }

  constexpr Token(TokenType type, std::string_view str, size_t line)
      : _type {type}
      , _str {str}
      , _line {line}
  {
  }

  constexpr inline TokenType type() const
  {
    return _type;
  }

  constexpr inline size_t line() const
  {
    return _line;
  }

  constexpr inline size_t length() const
  {
    return _str.length();
  }

  constexpr inline std::string_view string() const
  {
    return _str;
  }

private:
  TokenType _type;
  std::string_view _str;
  size_t _line;
};

std::ostream& operator<<(std::ostream& os, const Token& t);