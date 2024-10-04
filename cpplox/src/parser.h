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

  Token current() const;
  Token previous() const;

  bool hadError() const;
  bool inPanicMode() const;

  void enterPanicMode();
  void exitPanicMode();

  void setError(bool hasError);

private:
  std::shared_ptr<Scanner> scanner;

  Token _current;
  Token _previous;
  bool _hadError = false;
  bool _panicMode = false;
};