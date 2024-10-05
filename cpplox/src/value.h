#pragma once

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

struct Value
{
  ValueType type;
  union
  {
    bool boolean;
    double number;
    Obj* obj = nullptr;
  } as;
};

constexpr Value BOOL_VAL(bool val)
{
  return {ValueType::BOOL, {.boolean = (val)}};
}

constexpr Value NIL_VAL()
{
  return {ValueType::NIL, {.number = 0}};
}

constexpr Value NUMBER_VAL(double value)
{
  return {ValueType::NUMBER, {.number = (value)}};
}

constexpr Value OBJ_VAL(auto* object)
{
  return {ValueType::OBJ, {.obj = (Obj*)(object)}};
}

constexpr bool AS_BOOL(const Value& value)
{
  return value.as.boolean;
};

constexpr double AS_NUMBER(const Value& value)
{
  return value.as.number;
};

constexpr Obj* AS_OBJ(const Value& value)
{
  return value.as.obj;
};

constexpr bool IS_BOOL(const Value& value)
{
  return value.type == ValueType::BOOL;
}

constexpr bool IS_NIL(const Value& value)
{
  return value.type == ValueType::NIL;
}

constexpr bool IS_NUMBER(const Value& value)
{
  return value.type == ValueType::NUMBER;
}

constexpr bool IS_OBJ(const Value& value)
{
  return value.type == ValueType::OBJ;
}

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