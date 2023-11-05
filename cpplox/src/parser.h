#pragma once

#include <memory>

#include "scanner.h"

class Parser
{
public:
  explicit Parser(std::unique_ptr<Scanner> scanner);
  Parser(const Parser& other) = delete;
  Parser& operator=(const Parser& other) = delete;

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
  std::unique_ptr<Scanner> _scanner;
  Token _current;
  Token _previous;
  bool _hadError;
  bool _panicMode;
};
