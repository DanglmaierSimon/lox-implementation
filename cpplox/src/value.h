#pragma once

#include <cassert>
#include <string>
#include <variant>
#include <vector>

#include "common.h"
#include "obj.h"

using Value = std::variant<std::monostate, bool, double, Obj*>;

inline constexpr bool IS_BOOL(const Value& value)
{
  return std::holds_alternative<bool>(value);
}

inline constexpr bool IS_NIL(const Value& value)
{
  return std::holds_alternative<std::monostate>(value);
}

inline constexpr bool IS_NUMBER(const Value& value)
{
  return std::holds_alternative<double>(value);
}

constexpr bool IS_OBJ(const Value& value)
{
  return std::holds_alternative<Obj*>(value);
}

inline constexpr bool AS_BOOL(const Value& value)
{
  assert(IS_BOOL(value));
  return std::get<bool>(value);
}

inline constexpr double AS_NUMBER(const Value& value)
{
  assert(IS_NUMBER(value));
  return std::get<double>(value);
}

inline constexpr Obj* AS_OBJ(const Value& value)
{
  assert(IS_OBJ(value));
  const auto res = std::get<Obj*>(value);
  assert(res != nullptr);
  return res;
}

std::string toString(const Value& value);
inline constexpr bool valuesEqual(const Value& a, const Value& b)
{
  return a == b;
}

inline ObjType OBJ_TYPE(const Value& value)
{
  return AS_OBJ(value)->type();
}

inline bool isObjType(const Value& value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type() == type;
}