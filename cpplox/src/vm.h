#pragma once

#include <cstddef>

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

struct CallFrame
{
  ObjClosure* closure;
  uint8_t* ip;
  Value* slots;
};

struct VM
{
  CallFrame frames[FRAMES_MAX];
  int frameCount;

  Value stack[STACK_MAX];
  Value* stackTop;
  Table strings;

  ObjString* initString;

  Table globals;
  Obj* objects;
  ObjUpvalue* openUpValues;

  size_t bytesAllocated;
  size_t nextGC;

  int grayCount;
  int grayCapacity;
  Obj** grayStack;
};

enum class InterpretResult
{
  OK,
  COMPILER_ERROR,
  RUNTIME_ERROR
};

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();