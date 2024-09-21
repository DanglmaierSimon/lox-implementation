#pragma once

#include <cassert>
#include <cmath>
#include <string>

#include <fmt/printf.h>

#include "obj.h"

enum class ValueType
{
  BOOL,
  NIL,
  NUMBER,
  OBJ,
};

class Value final
{
public:
  inline explicit Value()
  {
    _type = ValueType::NIL;
    _as.number = 0;
  }

  inline explicit Value(bool val)
  {
    _type = ValueType::BOOL;
    this->_as.boolean = val;
  }

  inline explicit Value(double val)
  {
    _type = ValueType::NUMBER;
    _as.number = val;
  }

  inline explicit Value(Obj* object)
  {
    assert(object != nullptr);
    _type = ValueType::OBJ;
    _as.obj = object;
  }

  ~Value() = default;

  inline Value(const Value& v)
  {
    _type = v._type;
    _as = v._as;
  }

  inline Value& operator=(const Value& v)
  {
    if (this != &v) {
      _type = v._type;
      _as = v._as;
    }
    return *this;
  }

  //==================================================

  inline ValueType type() const { return _type; }

  //==================================================

  inline bool is_bool() const { return _type == ValueType::BOOL; }
  inline bool is_nil() const { return _type == ValueType::NIL; }
  inline bool is_number() const { return _type == ValueType::NUMBER; }
  inline bool is_obj() const { return _type == ValueType::OBJ; }

  // ==================================================

  inline bool as_bool() const
  {
    assert(is_bool());
    return _as.boolean;
  }

  inline double as_number() const
  {
    assert(is_number());
    return _as.number;
  }

  inline Obj* as_obj() const
  {
    assert(is_obj());
    return _as.obj;
  }

  inline bool isObjType(ObjType type) const
  {
    return is_obj() && as_obj()->type() == type;
  }

  inline ObjType OBJ_TYPE() const { return as_obj()->type(); }

  // ==================================================

private:
  ValueType _type = ValueType::NIL;
  union
  {
    bool boolean;
    double number;
    Obj* obj;
  } _as;
};

inline constexpr bool IS_BOOL(const Value& value)
{
  return value.is_bool();
}

inline constexpr bool IS_NIL(const Value& value)
{
  return value.is_nil();
}

inline constexpr bool IS_NUMBER(const Value& value)
{
  return value.is_number();
}

inline constexpr bool IS_OBJ(const Value& value)
{
  return value.is_obj();
}

inline constexpr bool AS_BOOL(const Value& value)
{
  assert(IS_BOOL(value));
  return value.as_bool();
}

inline constexpr double AS_NUMBER(const Value& value)
{
  assert(IS_NUMBER(value));
  return value.as_number();
}

inline constexpr Obj* AS_OBJ(const Value& value)
{
  assert(IS_OBJ(value));
  const auto res = (value.as_obj());
  assert(res != nullptr);
  return res;
}

inline ObjType OBJ_TYPE(const Value& value)
{
  return AS_OBJ(value)->type();
}

inline bool isObjType(const Value& value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type() == type;
}

inline std::string toString(const Value& value)
{
  switch (value.type()) {
    case ValueType::BOOL: {
      return value.as_bool() ? std::string {"true"} : std::string {"false"};
    }

    case ValueType::NIL:
      return std::string {"nil"};
    case ValueType::NUMBER:
      return fmt::sprintf("%g", value.as_number());
    case ValueType::OBJ:
      return value.as_obj()->toString();
  }
}

inline constexpr bool valuesEqual(const Value& a, const Value& b)
{
  if (a.type() != b.type()) {
    return false;
  }

  switch (a.type()) {
    case ValueType::BOOL:
      return a.as_bool() == b.as_bool();
    case ValueType::NIL:
      return true;
    case ValueType::NUMBER: {
      auto ai = a.as_number();
      auto bi = b.as_number();
      return fabs(ai - bi) <= ((fabs(ai) < fabs(bi) ? fabs(bi) : fabs(ai))
                               * std::numeric_limits<double>::epsilon());
    }
    case ValueType::OBJ:
      return a.as_obj() == b.as_obj();
  }
}

inline auto format_as(ValueType v)
{
  switch (v) {
    case ValueType::BOOL:
      return "ValueType::BOOL";
    case ValueType::NIL:
      return "ValueType::NIL";
    case ValueType::NUMBER:
      return "ValueType::NUMBER";
    case ValueType::OBJ:
      return "ValueType::OBJ";
  }
}