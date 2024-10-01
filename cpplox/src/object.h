#pragma once

#include <cstdint>
#include <vector>

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))

#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)

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
  struct Obj* next;
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
  ObjString* name;
};

struct ObjClosure
{
  Obj obj;
  ObjFunction* function;
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

static inline bool isObjType(Value value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}