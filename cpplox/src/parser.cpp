#include <iostream>
#include <string>
#include <string_view>

#include "parser.h"

#include <fmt/printf.h>

#include "scanner.h"

Parser::Parser(std::unique_ptr<Scanner> scanner)
    : _scanner {std::move(scanner)}
    , _hadError {false}
    , _panicMode {false}
{
}

Token Parser::current() const
{
  return _current;
}

Token Parser::previous() const
{
  return _previous;
}

bool Parser::panicMode() const
{
  return _panicMode;
}

void Parser::setPanicMode(bool panic)
{
  _panicMode = panic;
}

bool Parser::hadError() const
{
  return _hadError;
}

void Parser::setHadError(bool hadError)
{
  _hadError = hadError;
}

void Parser::advance()
{
  _previous = _current;

  while (true) {
    _current = _scanner->scanToken();
    if (current().type() != TokenType::ERROR) {
      break;
    }

    errorAtCurrent(current().string());
  }
}

void Parser::errorAtCurrent(std::string_view message)
{
  errorAt(current(), message);
}

void Parser::error(std::string_view message)
{
  errorAt(previous(), message);
}

void Parser::errorAt(Token token, std::string_view message)
{
  if (panicMode()) {
    // suppress subsequent errors in panic mode to prevent error cascades
    return;
  }

  enterPanicMode();
  std::cerr << fmt::sprintf("[line %d] Error", token.line());

  if (token.type() == TokenType::END_OF_FILE) {
    std::cerr << fmt::sprintf(" at end");
  } else if (token.type() == TokenType::ERROR) {
    // do nothing;
  } else {
    std::cerr << fmt::sprintf(" at '%s'", token.string());
  }

  std::cerr << fmt::sprintf(": %s\n", message);
  setHadError(true);
}

void Parser::consume(TokenType type, std::string_view message)
{
  if (current().type() == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

bool Parser::check(TokenType type) const
{
  return current().type() == type;
}

bool Parser::match(TokenType type)
{
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}

void Parser::synchronize()
{
  exitPanicMode();

  while (current().type() != TokenType::END_OF_FILE) {
    if (previous().type() == TokenType::SEMICOLON) {
      return;
    }

    switch (current().type()) {
      case TokenType::CLASS:
      case TokenType::FUN:
      case TokenType::VAR:
      case TokenType::FOR:
      case TokenType::IF:
      case TokenType::WHILE:
      case TokenType::PRINT:
      case TokenType::RETURN:
        return;
      default:
        break;  // do nothing
    }

    advance();
  }
}

void Parser::enterPanicMode()
{
  setPanicMode(true);
}

void Parser::exitPanicMode()
{
  setPanicMode(false);
}
