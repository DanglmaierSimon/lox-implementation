#include <string_view>
#include <unordered_map>

#include "lox/scanner.h"

namespace
{

constexpr bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

constexpr bool isAlpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
}  // namespace

Scanner::Scanner(std::string_view source)
    : _source {source}
{
  start = _source.begin();
  current = _source.begin();
  line = 1;
}

Scanner::Scanner(const Scanner& other)
    : _source(other._source)
    , line {other.line}
{
  const auto distanceFromBeginning =
      std::distance(other._source.cbegin(), other.start);
  const auto distanceFromStart = std::distance(other.start, other.current);

  start = _source.begin();
  std::advance(start, distanceFromBeginning);

  current = start;
  std::advance(current, distanceFromStart);
}

Scanner& Scanner::operator=(const Scanner& other)
{
  if (this != &other) {
    line = other.line;
    _source = other._source;
    const auto distanceFromBeginning =
        std::distance(other._source.cbegin(), other.start);
    const auto distanceFromStart = std::distance(other.start, other.current);

    start = _source.begin();
    std::advance(start, distanceFromBeginning);

    current = start;
    std::advance(current, distanceFromStart);
  }

  return *this;
}

bool Scanner::isAtEnd() const
{
  return *current == '\0';
}

char Scanner::advance()
{
  current++;
  return current[-1];
}

bool Scanner::match(char expected)
{
  if (isAtEnd()) {
    return false;
  }

  if (*current != expected) {
    return false;
  }

  current++;
  return true;
}

char Scanner::peek() const
{
  return *current;
}

char Scanner::peekNext() const
{
  if (isAtEnd()) {
    return '\0';
  }
  return current[1];
}

void Scanner::skipWhitespace()
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
        line++;
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

Token Scanner::makeToken(TokenType type) const
{
  return Token {
      type, {start, static_cast<unsigned long>(current - start)}, line};
}

Token Scanner::errorToken(std::string_view message) const
{
  return Token {TokenType::ERROR, message, line};
}

TokenType Scanner::identifierType()
{
  std::string_view str {start, static_cast<unsigned long>(current - start)};

  const std::unordered_map<std::string_view, TokenType> keywords {
      {"and", TokenType::AND},
      {"class", TokenType::CLASS},
      {"else", TokenType::ELSE},
      {"false", TokenType::FALSE},
      {"for", TokenType::FOR},
      {"fun", TokenType::FUN},
      {"if", TokenType::IF},
      {"nil", TokenType::NIL},
      {"or", TokenType::OR},
      {"print", TokenType::PRINT},
      {"return", TokenType::RETURN},
      {"super", TokenType::SUPER},
      {"this", TokenType::THIS},
      {"true", TokenType::TRUE},
      {"var", TokenType::VAR},
      {"while", TokenType::WHILE},
  };

  if (keywords.count(str) > 0) {
    return keywords.at(str);
  }

  return TokenType::IDENTIFIER;
}

Token Scanner::string_()
{
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      line++;
    }
    advance();
  }

  if (isAtEnd()) {
    return errorToken("Unterminated string.");
  }

  // closing quote
  advance();
  return makeToken(TokenType::STRING);
}

Token Scanner::number()
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

  return makeToken(TokenType::NUMBER);
}

Token Scanner::identifier()
{
  while (isAlpha(peek()) || isDigit(peek())) {
    advance();
  }

  return makeToken(identifierType());
}

Token Scanner::scanToken()
{
  skipWhitespace();
  start = current;

  if (isAtEnd()) {
    return makeToken(TokenType::END_OF_FILE);
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
      return makeToken(TokenType::LEFT_PAREN);
    case ')':
      return makeToken(TokenType::RIGHT_PAREN);
    case '{':
      return makeToken(TokenType::LEFT_BRACE);
    case '}':
      return makeToken(TokenType::RIGHT_BRACE);
    case ';':
      return makeToken(TokenType::SEMICOLON);
    case ',':
      return makeToken(TokenType::COMMA);
    case '.':
      return makeToken(TokenType::DOT);
    case '-':
      return makeToken(TokenType::MINUS);
    case '+':
      return makeToken(TokenType::PLUS);
    case '/':
      return makeToken(TokenType::SLASH);
    case '*':
      return makeToken(TokenType::STAR);

      // Two character tokens
    case '!':
      return makeToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    case '=':
      return makeToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    case '<':
      return makeToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    case '>':
      return makeToken(match('=') ? TokenType::GREATER_EQUAL
                                  : TokenType::GREATER);

    // string literals
    case '"':
      return string_();
  }

  return errorToken("Unexpected character.");
}

std::ostream& operator<<(std::ostream& os, const Token& t)
{
  os << "Token { " << t.type() << "; " << t.string() << "; " << t.line()
     << " }";

  return os;
}

std::ostream& operator<<(std::ostream& os, TokenType t)
{
  switch (t) {
    case TokenType::LEFT_PAREN: {
      os << "TokenType::LEFT_PAREN";
      break;
    }
    case TokenType::RIGHT_PAREN: {
      os << "TokenType::RIGHT_PAREN";
      break;
    }
    case TokenType::LEFT_BRACE: {
      os << "TokenType::LEFT_BRACE";
      break;
    }
    case TokenType::RIGHT_BRACE: {
      os << "TokenType::RIGHT_BRACE";
      break;
    }
    case TokenType::COMMA: {
      os << "TokenType::COMMA";
      break;
    }
    case TokenType::DOT: {
      os << "TokenType::DOT";
      break;
    }
    case TokenType::MINUS: {
      os << "TokenType::MINUS";
      break;
    }
    case TokenType::PLUS: {
      os << "TokenType::PLUS";
      break;
    }
    case TokenType::SEMICOLON: {
      os << "TokenType::SEMICOLON";
      break;
    }
    case TokenType::SLASH: {
      os << "TokenType::SLASH";
      break;
    }
    case TokenType::STAR: {
      os << "TokenType::STAR";
      break;
    }
    case TokenType::BANG: {
      os << "TokenType::BANG";
      break;
    }
    case TokenType::BANG_EQUAL: {
      os << "TokenType::BANG_EQUAL";
      break;
    }
    case TokenType::EQUAL: {
      os << "TokenType::EQUAL";
      break;
    }
    case TokenType::EQUAL_EQUAL: {
      os << "TokenType::EQUAL_EQUAL";
      break;
    }
    case TokenType::GREATER: {
      os << "TokenType::GREATER";
      break;
    }
    case TokenType::GREATER_EQUAL: {
      os << "TokenType::GREATER_EQUAL";
      break;
    }
    case TokenType::LESS: {
      os << "TokenType::LESS";
      break;
    }
    case TokenType::LESS_EQUAL: {
      os << "TokenType::LESS_EQUAL";
      break;
    }
    case TokenType::IDENTIFIER: {
      os << "TokenType::IDENTIFIER";
      break;
    }
    case TokenType::STRING: {
      os << "TokenType::STRING";
      break;
    }
    case TokenType::NUMBER: {
      os << "TokenType::NUMBER";
      break;
    }
    case TokenType::AND: {
      os << "TokenType::AND";
      break;
    }
    case TokenType::CLASS: {
      os << "TokenType::CLASS";
      break;
    }
    case TokenType::ELSE: {
      os << "TokenType::ELSE";
      break;
    }
    case TokenType::FALSE: {
      os << "TokenType::FALSE";
      break;
    }
    case TokenType::FOR: {
      os << "TokenType::FOR";
      break;
    }
    case TokenType::FUN: {
      os << "TokenType::FUN";
      break;
    }
    case TokenType::IF: {
      os << "TokenType::IF";
      break;
    }
    case TokenType::NIL: {
      os << "TokenType::NIL";
      break;
    }
    case TokenType::OR: {
      os << "TokenType::OR";
      break;
    }
    case TokenType::PRINT: {
      os << "TokenType::PRINT";
      break;
    }
    case TokenType::RETURN: {
      os << "TokenType::RETURN";
      break;
    }
    case TokenType::SUPER: {
      os << "TokenType::SUPER";
      break;
    }
    case TokenType::THIS: {
      os << "TokenType::THIS";
      break;
    }
    case TokenType::TRUE: {
      os << "TokenType::TRUE";
      break;
    }
    case TokenType::VAR: {
      os << "TokenType::VAR";
      break;
    }
    case TokenType::WHILE: {
      os << "TokenType::WHILE";
      break;
    }
    case TokenType::ERROR: {
      os << "TokenType::ERROR";
      break;
    }
    case TokenType::END_OF_FILE: {
      os << "TokenType::END_OF_FILE";
      break;
    }
  }

  return os;
}
