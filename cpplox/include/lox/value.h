#pragma once

#include <string>
#include <variant>
#include <vector>

#include "common.h"
#include "lox/objects/obj.h"

class Value
{
  friend std::string toString(Value);

public:
  constexpr explicit Value() = default;

  constexpr Value(const Value& other)
      : _value(other._value)
      , _isConstant(other.isConst())
  {
  }

  Value operator=(const Value& other)
  {
    if (this != &other) {
      _isConstant = other.isConst();
      _value = other._value;
    }

    return *this;
  }

  constexpr explicit Value(bool v, bool isConstant = false)
      : _value {v}
      , _isConstant {isConstant}
  {
  }

  constexpr explicit Value(double v, bool isConstant = false)
      : _value {v}
      , _isConstant {isConstant}
  {
  }

  constexpr explicit Value(Obj* v, bool isConstant = false)
      : _value {v}
      , _isConstant {isConstant}
  {
  }

  constexpr bool operator==(const Value& other)
  {
    return other._value == this->_value;
  }

  constexpr inline bool isConst() const
  {
    return _isConstant;
  }

  template<typename T>
  inline constexpr bool is() const
  {
    return std::holds_alternative<T>(_value);
  }

  template<typename T>
  inline constexpr T as() const
  {
    return std::get<T>(_value);
  }

private:
  std::variant<std::monostate, bool, double, Obj*> _value;
  bool _isConstant = false;
};

inline bool IS_BOOL(Value value)
{
  return value.is<bool>();
}

inline bool IS_NIL(Value value)
{
  return value.is<std::monostate>();
}

inline bool IS_NUMBER(Value value)
{
  return value.is<double>();
}

inline bool IS_OBJ(Value value)
{
  return value.is<Obj*>();
}

inline bool AS_BOOL(Value value)
{
  assert(IS_BOOL(value));
  return value.as<bool>();
}

inline double AS_NUMBER(Value value)
{
  assert(IS_NUMBER(value));
  return value.as<double>();
}

inline Obj* AS_OBJ(Value value)
{
  assert(IS_OBJ(value));
  return value.as<Obj*>();
}

std::string toString(Value value);
bool valuesEqual(Value a, Value b);

inline ObjType OBJ_TYPE(Value value)
{
  return AS_OBJ(value)->type();
}

inline bool isObjType(Value value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type() == type;
}