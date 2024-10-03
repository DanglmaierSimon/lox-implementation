

#include <cassert>
#include <cstdio>

#include "parser.h"

#include "scanner.h"

Parser::Parser(std::shared_ptr<Scanner> _scanner)
    : scanner(std::move(_scanner))
{
}

void Parser::errorAt(Token* token, const char* message)
{
  assert(token != nullptr);

  if (panicMode) {
    // suppress subsequent errors in panic mode to prevent error cascades
    return;
  }

  panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::END_OF_FILE) {
    fprintf(stderr, " at end");
  } else if (token->type == TokenType::ERROR_TOKEN) {
    // do nothing;
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  hadError = true;
}

void Parser::errorAtCurrent(const char* message)
{
  errorAt(&current, message);
}

void Parser::error(const char* message)
{
  errorAt(&previous, message);
}

void Parser::advance()
{
  previous = current;

  while (true) {
    current = scanner->scanToken();
    if (current.type != TokenType::ERROR_TOKEN) {
      break;
    }

    errorAtCurrent(current.start);
  }
}

void Parser::consume(TokenType type, const char* message)
{
  if (current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

bool Parser::check(TokenType type)
{
  return current.type == type;
}

bool Parser::match(TokenType type)
{
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}