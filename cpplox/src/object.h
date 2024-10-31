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
  Obj* next = nullptr;
};

struct ObjString : Obj
{
  int length;
  char* chars = nullptr;
  uint32_t hash;
};

struct ObjUpvalue : Obj
{
  Value* location = nullptr;  // non-owning, do not delete -> multiple closures
                              // can close over the same variable
  ObjUpvalue* nextupval;
  Value closed;
};

struct ObjFunction : Obj
{
  int arity;
  int upvalueCount;

  Chunk chunk;
  ObjString* name = nullptr;
};

struct ObjClosure : Obj
{
  ObjFunction* function = nullptr;
  ObjUpvalue** upvalues = nullptr;
  int upvalueCount;
};

typedef Value (*NativeFn)(int argCount, Value* args);

struct ObjNative : Obj
{
  NativeFn function;
};

struct ObjClass : Obj
{
  ObjString* name;
  Table methods;
};

struct ObjInstance : Obj
{
  ObjClass* klass;
  Table fields;
};

struct ObjBoundMethod : Obj
{
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
  return value.is_obj() && value.as_obj()->type == type;
}

constexpr ObjType OBJ_TYPE(Value const& value)
{
  assert(value.is_obj());
  assert(value.as_obj() != nullptr);
  return (value.as_obj()->type);
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
  return ((ObjString*)value.as_obj());
}

constexpr auto AS_CSTRING(Value const& value)
{
  assert(IS_STRING(value));
  return (((ObjString*)value.as_obj())->chars);
}

constexpr auto AS_FUNCTION(Value const& value)
{
  assert(IS_FUNCTION(value));
  return ((ObjFunction*)value.as_obj());
}

constexpr auto AS_NATIVE(Value const& value)
{
  assert(IS_NATIVE(value));
  return (((ObjNative*)value.as_obj())->function);
}

constexpr auto AS_CLOSURE(Value const& value)
{
  assert(IS_CLOSURE(value));
  return ((ObjClosure*)value.as_obj());
}

constexpr auto AS_CLASS(Value const& value)
{
  assert(IS_CLASS(value));
  return ((ObjClass*)value.as_obj());
}

constexpr auto AS_INSTANCE(Value const& value)
{
  assert(IS_INSTANCE(value));
  return ((ObjInstance*)value.as_obj());
}

constexpr auto AS_BOUND_METHOD(Value const& value)
{
  assert(IS_BOUND_METHOD(value));
  return ((ObjBoundMethod*)value.as_obj());
}
