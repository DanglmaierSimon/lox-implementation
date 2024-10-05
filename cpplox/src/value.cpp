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
  switch (value.type) {
    case ValueType::BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case ValueType::NIL:
      printf("nil");
      break;
    case ValueType::NUMBER:
      printf("%g", AS_NUMBER(value));
      break;
    case ValueType::OBJ:
      printObject(value);
      break;
  }
}

bool valuesEqual(Value const& a, Value const& b)
{
  if (a.type != b.type) {
    return false;
  }

  switch (a.type) {
    case ValueType::BOOL:
      return AS_BOOL(a) == AS_BOOL(b);
    case ValueType::NIL:
      return true;
    case ValueType::NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);
    case ValueType::OBJ:
      return AS_OBJ(a) == AS_OBJ(b);
  }
}