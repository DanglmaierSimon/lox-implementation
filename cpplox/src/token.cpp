#include "token.h"

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