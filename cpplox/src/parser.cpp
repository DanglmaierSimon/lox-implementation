

#include <cassert>
#include <cstdio>

#include "parser.h"

#include "scanner.h"

Parser::Parser(std::shared_ptr<Scanner> _scanner)
    : scanner(std::move(_scanner))
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

bool Parser::hadError() const
{
  return _hadError;
}

bool Parser::inPanicMode() const
{
  return _panicMode;
}

void Parser::enterPanicMode()
{
  _panicMode = true;
}

void Parser::exitPanicMode()
{
  _panicMode = false;
}

void Parser::setError(bool hasError)
{
  _hadError = hasError;
}

void Parser::errorAt(Token* token, const char* message)
{
  assert(token != nullptr);

  if (inPanicMode()) {
    // suppress subsequent errors in panic mode to prevent error cascades
    return;
  }

  _panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::END_OF_FILE) {
    fprintf(stderr, " at end");
  } else if (token->type == TokenType::ERROR_TOKEN) {
    // do nothing;
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  _hadError = true;
}

void Parser::errorAtCurrent(const char* message)
{
  errorAt(&_current, message);
}

void Parser::error(const char* message)
{
  errorAt(&_previous, message);
}

void Parser::advance()
{
  _previous = _current;

  while (true) {
    _current = scanner->scanToken();
    if (current().type != TokenType::ERROR_TOKEN) {
      break;
    }

    errorAtCurrent(_current.start);
  }
}

void Parser::consume(TokenType type, const char* message)
{
  if (current().type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

bool Parser::check(TokenType type)
{
  return current().type == type;
}

bool Parser::match(TokenType type)
{
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}