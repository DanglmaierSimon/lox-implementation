#pragma once

#include <iostream>
#include <optional>
#include <string_view>

#include "token.h"

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
  std::optional<Token> skipWhitespace();

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
