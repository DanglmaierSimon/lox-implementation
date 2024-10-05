#pragma once

#include <cassert>
#include <cstdint>

#include "chunk.h"
#include "table.h"
#include "value.h"

enum ObjType
{
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_UPVALUE,
  OBJ_CLASS,
  OBJ_INSTANCE,
  OBJ_BOUND_METHOD,
};

struct Obj
{
  ObjType type;
  bool isMarked;
  struct Obj* next = nullptr;
};

struct ObjString
{
  Obj obj;
  int length;
  char* chars = nullptr;
  uint32_t hash;
};

struct ObjUpvalue
{
  Obj obj;
  Value* location = nullptr;  // non-owning, do not delete -> multiple closures
                              // can close over the same variable
  ObjUpvalue* next;
  Value closed;
};

struct ObjFunction
{
  Obj obj;
  int arity;
  int upvalueCount;

  Chunk chunk;
  ObjString* name = nullptr;
};

struct ObjClosure
{
  Obj obj;
  ObjFunction* function = nullptr;
  ObjUpvalue** upvalues = nullptr;
  int upvalueCount;
};

typedef Value (*NativeFn)(int argCount, Value* args);

struct ObjNative
{
  Obj obj;
  NativeFn function;
};

struct ObjClass
{
  Obj obj;
  ObjString* name;
  Table methods;
};

struct ObjInstance
{
  Obj obj;
  ObjClass* klass;
  Table fields;
};

struct ObjBoundMethod
{
  Obj obj;
  Value receiver;
  ObjClosure* method;
};

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjInstance* newInstance(ObjClass* klass);
ObjClass* newClass(ObjString* name);
ObjUpvalue* newUpvalue(Value* slot);
ObjClosure* newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

constexpr bool isObjType(const Value& value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

constexpr ObjType OBJ_TYPE(Value const& value)
{
  assert(IS_OBJ(value));
  assert(AS_OBJ(value) != nullptr);
  return (AS_OBJ(value)->type);
}

constexpr bool IS_STRING(Value const& value)
{
  return isObjType(value, OBJ_STRING);
}

constexpr bool IS_FUNCTION(Value const& value)
{
  return isObjType(value, OBJ_FUNCTION);
}

constexpr bool IS_NATIVE(Value const& value)
{
  return isObjType(value, OBJ_NATIVE);
}

constexpr bool IS_CLOSURE(Value const& value)
{
  return isObjType(value, OBJ_CLOSURE);
}

constexpr bool IS_CLASS(Value const& value)
{
  return isObjType(value, OBJ_CLASS);
}

constexpr bool IS_INSTANCE(Value const& value)
{
  return isObjType(value, OBJ_INSTANCE);
}

constexpr bool IS_BOUND_METHOD(Value const& value)
{
  return isObjType(value, OBJ_BOUND_METHOD);
}

constexpr auto AS_STRING(Value const& value)
{
  assert(IS_STRING(value));
  return ((ObjString*)AS_OBJ(value));
}

constexpr auto AS_CSTRING(Value const& value)
{
  assert(IS_STRING(value));
  return (((ObjString*)AS_OBJ(value))->chars);
}

constexpr auto AS_FUNCTION(Value const& value)
{
  assert(IS_FUNCTION(value));
  return ((ObjFunction*)AS_OBJ(value));
}

constexpr auto AS_NATIVE(Value const& value)
{
  assert(IS_NATIVE(value));
  return (((ObjNative*)AS_OBJ(value))->function);
}

constexpr auto AS_CLOSURE(Value const& value)
{
  assert(IS_CLOSURE(value));
  return ((ObjClosure*)AS_OBJ(value));
}

constexpr auto AS_CLASS(Value const& value)
{
  assert(IS_CLASS(value));
  return ((ObjClass*)AS_OBJ(value));
}

constexpr auto AS_INSTANCE(Value const& value)
{
  assert(IS_INSTANCE(value));
  return ((ObjInstance*)AS_OBJ(value));
}

constexpr auto AS_BOUND_METHOD(Value const& value)
{
  assert(IS_BOUND_METHOD(value));
  return ((ObjBoundMethod*)AS_OBJ(value));
}
