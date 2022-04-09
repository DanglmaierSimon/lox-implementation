#pragma once

#include <deque>

#include "lox/token.h"

class Parser
{
public:
  explicit Parser(std::deque<Token> tokens);

  // getters
  Token current() const;
  Token previous() const;
  bool panicMode() const;

  bool hadError() const;

  // setters
  void enterPanicMode();
  void exitPanicMode();
  void setHadError(bool hadError);

  // error functions
  void errorAtCurrent(std::string_view message);
  void error(std::string_view message);
  void errorAt(Token token, std::string_view message);

  void advance();
  void consume(TokenType type, std::string_view message);
  bool check(TokenType type) const;
  bool match(TokenType type);

  void synchronize();

private:
  void setPanicMode(bool panic);

private:
  std::deque<Token> _tokens;
  Token _current;
  Token _previous;
  bool _hadError;
  bool _panicMode;
};
