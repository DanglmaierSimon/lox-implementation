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

inline auto format_as(ObjType t)
{
  switch (t) {
    case ObjType::CLOSURE:
      return "ObjType::CLOSURE";
    case ObjType::FUNCTION:
      return "ObjType::FUNCTION";
    case ObjType::NATIVE:
      return "ObjType::NATIVE";
    case ObjType::STRING:
      return "ObjType::STRING";
    case ObjType::UPVALUE:
      return "ObjType::UPVALUE";
    case ObjType::CLASS:
      return "ObjType::CLASS";
    case ObjType::INSTANCE:
      return "ObjType::INSTANCE";
    case ObjType::BOUND_METHOD:
      return "ObjType::BOUND_METHOD";
  }
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
