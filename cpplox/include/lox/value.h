#pragma once

#include <cassert>
#include <string>
#include <variant>
#include <vector>

#include "common.h"
#include "lox/objects/obj.h"

using Value = std::variant<std::monostate, bool, double, Obj*>;

inline constexpr bool IS_BOOL(Value value)
{
  return std::holds_alternative<bool>(value);
}

inline constexpr bool IS_NIL(Value value)
{
  return std::holds_alternative<std::monostate>(value);
}

inline constexpr bool IS_NUMBER(Value value)
{
  return std::holds_alternative<double>(value);
}

constexpr bool IS_OBJ(Value value)
{
  return std::holds_alternative<Obj*>(value);
}

inline constexpr bool AS_BOOL(Value value)
{
  assert(IS_BOOL(value));
  return std::get<bool>(value);
}

inline constexpr double AS_NUMBER(Value value)
{
  assert(IS_NUMBER(value));
  return std::get<double>(value);
}

inline constexpr Obj* AS_OBJ(Value value)
{
  assert(IS_OBJ(value));
  return std::get<Obj*>(value);
}

std::string toString(Value value);
void printValue(Value value);
bool valuesEqual(Value a, Value b);

inline ObjType OBJ_TYPE(Value value)
{
  return AS_OBJ(value)->type();
}

inline bool isObjType(Value value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type() == type;
}