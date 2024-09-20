module;

#include <iostream>
#include <string_view>

export module token;

export enum class TokenType {
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  BREAK,
  CLASS,
  CONTINUE,
  ELSE,
  FALSE,
  FOR,
  FUN,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  // Special tokens
  ERROR,
  END_OF_FILE,
};

export inline std::ostream& operator<<(std::ostream& os, TokenType t)
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
    case TokenType::BREAK: {
      os << "TokenType::BREAK";
      break;
    }
    case TokenType::CONTINUE: {
      os << "TokenType::CONTINUE";
      break;
    }
  }

  return os;
}

export class Token
{
public:
  Token() = default;

  inline Token(TokenType type, std::string_view str, size_t line)
      : _type {type}
      , _str {str}
      , _line {line}
  {
  }

  inline TokenType type() const { return _type; }

  inline size_t line() const { return _line; }

  inline size_t length() const { return _str.length(); }

  inline std::string_view string() const { return _str; }

private:
  TokenType _type;
  std::string_view _str;
  size_t _line;
};

export inline std::ostream& operator<<(std::ostream& os, const Token& t)
{
  os << "Token { " << t.type() << "; " << t.string() << "; " << t.line()
     << " }";

  return os;
}