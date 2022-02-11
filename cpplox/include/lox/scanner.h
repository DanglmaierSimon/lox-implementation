#pragma once

#include <iostream>
#include <string_view>

enum class TokenType
{
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

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
  WHILE,

  // Special tokens
  ERROR,
  END_OF_FILE,
};

std::ostream& operator<<(std::ostream& os, TokenType t);

class Token
{
public:
  Token() = default;

  Token(TokenType type, std::string_view str, int line)
      : _type {type}
      , _str {str}
      , _line {line}
  {
  }

  inline TokenType type() const
  {
    return _type;
  }

  inline int line() const
  {
    return _line;
  }

  inline auto length() const
  {
    return _str.length();
  }

  inline std::string_view string() const
  {
    return _str;
  }

private:
  TokenType _type;
  std::string_view _str;
  int _line = 0;
};

std::ostream& operator<<(std::ostream& os, const Token& t);

class Scanner
{
public:
  explicit Scanner(std::string_view source);
  Scanner(const Scanner& other);
  Scanner& operator=(const Scanner& other);

  Token scanToken();

private:
  bool isAtEnd() const;
  char advance();
  bool match(char expected);
  char peek() const;
  char peekNext() const;
  void skipWhitespace();

  Token makeToken(TokenType type) const;
  Token errorToken(std::string_view message) const;
  TokenType identifierType();

  Token string_();
  Token number();
  Token identifier();

private:
  std::string_view _source;
  const char* start = nullptr;
  const char* current = nullptr;
  int line = 1;
};
