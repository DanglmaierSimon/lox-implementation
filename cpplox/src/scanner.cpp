#include "scanner.h"

#include <stdio.h>
#include <string.h>

#include "common.h"

struct Scanner
{
  const char* start;
  const char* current;
  int line;
};

Scanner scanner;

static bool isAtEnd()
{
  return *scanner.current == '\0';
}

static char advance()
{
  scanner.current++;
  return scanner.current[-1];
}

static bool match(char expected)
{
  if (isAtEnd()) {
    return false;
  }

  if (*scanner.current != expected) {
    return false;
  }

  scanner.current++;
  return true;
}

static char peek()
{
  return *scanner.current;
}

static char peekNext()
{
  if (isAtEnd()) {
    return '\0';
  }
  return scanner.current[1];
}

static void skipWhitespace()
{
  while (true) {
    char c = peek();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
      case '\n':
        scanner.line++;
        advance();
        break;
      case '/':
        if (peekNext() == '/') {
          // comments go to the end of the line
          while (peek() != '\n' && !isAtEnd()) {
            advance();
          }
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

static bool isAlpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static Token makeToken(TokenType type)
{
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

static Token errorToken(const char* message)
{
  Token token;
  token.type = ERROR_TOKEN;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

static TokenType checkKeyword(int start,
                              int length,
                              const char* rest,
                              TokenType type)
{
  if (scanner.current - scanner.start == start + length
      && memcmp(scanner.start + start, rest, length) == 0)
  {
    return type;
  }
  return IDENTIFIER;
}

static TokenType identifierType()
{
  switch (scanner.start[0]) {
    case 'a':
      return checkKeyword(1, 2, "nd", AND);
    case 'c':
      return checkKeyword(1, 4, "lass", CLASS);
    case 'e':
      return checkKeyword(1, 3, "lse", ELSE);
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a':
            return checkKeyword(2, 3, "lse", FALSE);
          case 'o':
            return checkKeyword(2, 1, "r", FOR);
          case 'u':
            return checkKeyword(2, 1, "n", FUN);
        }
      }
      break;
    case 'i':
      return checkKeyword(1, 1, "f", IF);
    case 'n':
      return checkKeyword(1, 2, "il", NIL);
    case 'o':
      return checkKeyword(1, 1, "r", OR);
    case 'p':
      return checkKeyword(1, 4, "rint", PRINT);
    case 'r':
      return checkKeyword(1, 5, "eturn", RETURN);
    case 's':
      return checkKeyword(1, 4, "uper", SUPER);
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h':
            return checkKeyword(2, 2, "is", THIS);
          case 'r':
            return checkKeyword(2, 2, "ue", TRUE);
        }
      }
      break;
    case 'v':
      return checkKeyword(1, 2, "ar", VAR);
    case 'w':
      return checkKeyword(1, 4, "hile", WHILE);
  }

  return IDENTIFIER;
}

static Token string()
{
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      scanner.line++;
    }
    advance();
  }

  if (isAtEnd()) {
    return errorToken("Unterminated string.");
  }

  // closing quote
  advance();
  return makeToken(STRING);
}

static Token number()
{
  while (isDigit(peek())) {
    advance();
  }

  // Look for fractional part
  if (peek() == '.' && isDigit(peekNext())) {
    // consume the dot
    advance();

    while (isDigit(peek())) {
      advance();
    }
  }

  return makeToken(NUMBER);
}

static Token identifier()
{
  while (isAlpha(peek()) || isDigit(peek())) {
    advance();
  }

  return makeToken(identifierType());
}

void initScanner(const char* source)
{
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

Token scanToken()
{
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd()) {
    return makeToken(EOF_TOKEN);
  }

  const char c = advance();

  if (isAlpha(c)) {
    return identifier();
  }

  if (isDigit(c)) {
    return number();
  }

  switch (c) {
      // One character tokens
    case '(':
      return makeToken(LEFT_PAREN);
    case ')':
      return makeToken(RIGHT_PAREN);
    case '{':
      return makeToken(LEFT_BRACE);
    case '}':
      return makeToken(RIGHT_BRACE);
    case ';':
      return makeToken(SEMICOLON);
    case ',':
      return makeToken(COMMA);
    case '.':
      return makeToken(DOT);
    case '-':
      return makeToken(MINUS);
    case '+':
      return makeToken(PLUS);
    case '/':
      return makeToken(SLASH);
    case '*':
      return makeToken(STAR);

      // Two character tokens
    case '!':
      return makeToken(match('=') ? BANG_EQUAL : BANG);
    case '=':
      return makeToken(match('=') ? EQUAL_EQUAL : EQUAL);
    case '<':
      return makeToken(match('=') ? LESS_EQUAL : LESS);
    case '>':
      return makeToken(match('=') ? GREATER_EQUAL : GREATER);

    // string literals
    case '"':
      return string();
  }

  return errorToken("Unexpected character.");
}