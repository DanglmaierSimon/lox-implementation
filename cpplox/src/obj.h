#pragma once

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
