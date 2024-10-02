#pragma once

#include "token.h"

class Scanner
{
public:
  Scanner(const char* source);

  Token scanToken();

private:
  char advance();
  bool isAtEnd() const;
  bool match(char expected);
  char peek() const;
  char peekNext() const;
  void skipWhitespace();

  Token makeToken(TokenType type);
  Token errorToken(const char* message);

  TokenType checkKeyword(int start,
                         int length,
                         const char* rest,
                         TokenType type) const;

  TokenType identifierType() const;

  Token string();
  Token number();
  Token identifier();

  const char* start = nullptr;
  const char* current = nullptr;
  int line = 1;
};
