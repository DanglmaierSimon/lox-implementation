#pragma once

#include <memory>

#include "token.h"

class Scanner;

class Parser
{
public:
  explicit Parser(std::shared_ptr<Scanner> _scanner);

  void errorAt(Token* token, const char* message);
  void errorAtCurrent(const char* message);
  void error(const char* message);

  void advance();
  void consume(TokenType type, const char* message);
  bool check(TokenType type);
  bool match(TokenType type);

private:
  std::shared_ptr<Scanner> scanner;

public:
  Token current;
  Token previous;
  bool hadError = false;
  bool panicMode = false;
};