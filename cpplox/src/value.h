#pragma once

#include <cassert>
#include <cstddef>

struct Obj;
struct ObjString;

enum class ValueType
{
  BOOL,
  NIL,
  NUMBER,
  OBJ
};

class Value
{
public:
  constexpr Value()
  {
    _type = ValueType::NIL;
    as = {.number = 0};
  }

  constexpr explicit Value(bool val)
  {
    _type = ValueType::BOOL;
    as = {.boolean = (val)};
  }

  constexpr explicit Value(double number)
  {
    _type = ValueType::NUMBER;
    as = {.number = (number)};
  }

  constexpr explicit Value(Obj* obj)
  {
    _type = ValueType::OBJ;
    as = {.obj = obj};
  }

  constexpr bool is_bool() const { return _type == ValueType::BOOL; }

  constexpr bool is_nil() const { return _type == ValueType::NIL; }

  constexpr bool is_number() const { return _type == ValueType::NUMBER; }

  constexpr bool is_obj() const { return _type == ValueType::OBJ; }

  constexpr bool as_bool() const
  {
    assert(is_bool());
    return as.boolean;
  }

  constexpr double as_number() const
  {
    assert(is_number());
    return as.number;
  };

  constexpr Obj* as_obj() const
  {
    assert(is_obj());
    return as.obj;
  };

  constexpr ValueType type() const { return _type; }

private:
  ValueType _type;
  union
  {
    bool boolean;
    double number;
    Obj* obj = nullptr;
  } as;
};

struct ValueArray
{
  size_t capacity = 0;
  size_t count = 0;
  Value* values = nullptr;
};

bool valuesEqual(const Value& a, const Value& b);

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);