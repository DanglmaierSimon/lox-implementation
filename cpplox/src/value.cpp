#include <cstdio>
#include <cstring>

#include "value.h"

#include "memory.h"
#include "object.h"

void initValueArray(ValueArray* array)
{
  array->values = nullptr;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray* array, Value value)
{
  if (array->capacity < array->count + 1) {
    auto oldcap = array->capacity;
    array->capacity = GROW_CAPACITY(oldcap);
    array->values = GROW_ARRAY(array->values, oldcap, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void freeValueArray(ValueArray* array)
{
  FREE_ARRAY<Value>(array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value)
{
  switch (value.type()) {
    case ValueType::BOOL:
      printf(value.as_bool() ? "true" : "false");
      break;
    case ValueType::NIL:
      printf("nil");
      break;
    case ValueType::NUMBER:
      printf("%g", value.as_number());
      break;
    case ValueType::OBJ:
      printObject(value);
      break;
  }
}

bool valuesEqual(Value const& a, Value const& b)
{
  if (a.type() != b.type()) {
    return false;
  }

  switch (a.type()) {
    case ValueType::BOOL:
      return a.as_bool() == b.as_bool();
    case ValueType::NIL:
      return true;
    case ValueType::NUMBER:
      return a.as_number() == b.as_number();
    case ValueType::OBJ:
      return a.as_obj() == b.as_obj();
  }
}