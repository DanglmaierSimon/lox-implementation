#pragma once

#include <ostream>
#include <string>

enum class ObjType
{
  CLOSURE,
  FUNCTION,
  NATIVE,
  STRING,
  UPVALUE,
  CLASS,
  INSTANCE,
  BOUND_METHOD,
};

inline std::ostream& operator<<(std::ostream& os, ObjType t)
{
  switch (t) {
    case ObjType::CLOSURE:
      os << "ObjType::CLOSURE";
      break;
    case ObjType::FUNCTION:
      os << "ObjType::FUNCTION";
      break;
    case ObjType::NATIVE:
      os << "ObjType::NATIVE";
      break;
    case ObjType::STRING:
      os << "ObjType::STRING";
      break;
    case ObjType::UPVALUE:
      os << "ObjType::UPVALUE";
      break;
    case ObjType::CLASS:
      os << "ObjType::CLASS";
      break;
    case ObjType::INSTANCE:
      os << "ObjType::INSTANCE";
      break;
    case ObjType::BOUND_METHOD:
      os << "ObjType::BOUND_METHOD";
      break;

    default:
      __builtin_unreachable();
  }

  return os;
}

class Obj
{
public:
  Obj() = default;
  virtual ~Obj() = default;

  virtual std::string toString() const = 0;
  virtual ObjType type() const = 0;

  bool isMarked() const;
  void setIsMarked(bool marked);

  Obj* nextObj() const;
  void setNextObj(Obj* next);

private:
  bool _isMarked = false;
  Obj* _nextObj = nullptr;
};
