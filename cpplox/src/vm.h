#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <string_view>

#include "chunk.h"
#include "objclass.h"
#include "objclosure.h"
#include "objnative.h"
#include "table.h"
#include "value.h"

constexpr size_t FRAMES_MAX = 64u;
constexpr size_t STACK_MAX = (FRAMES_MAX * UINT8_COUNT);

class MemoryManager;

enum class InterpretResult
{
  OK,
  COMPILE_ERROR,
  RUNTIME_ERROR
};

inline std::ostream& operator<<(std::ostream& os, InterpretResult result)
{
  switch (result) {
    case InterpretResult::OK:
      return os << "InterpretResult::OK";

    case InterpretResult::COMPILE_ERROR:
      return os << "InterpretResult::COMPILE_ERROR";

    case InterpretResult::RUNTIME_ERROR:
      return os << "InterpretResult::RUNTIME_ERROR";
  }

  return os;
}

struct CallFrame
{
  ObjClosure* closure = nullptr;
  const uint8_t* ip;
  Value* slots = nullptr;
};

class VM
{
  friend class MemoryManager;

public:
  explicit VM();
  virtual ~VM();

  InterpretResult interpret(std::string_view source);
  void push(Value value);
  Value pop();

private:
  void resetStack();
  void runtimeError(std::string msg);
  void defineNative(std::string name, NativeFn function);
  Value peek(int distance);
  bool call(ObjClosure* closure, int argCount);
  bool callValue(Value callee, int argCount);
  bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount);
  bool invoke(ObjString* name, int argCount);
  bool bindMethod(ObjClass* klass, ObjString* name);
  InterpretResult run();
  void concatenate();
  ObjUpvalue* captureUpvalue(Value* local);
  void closeUpvalues(Value* last);
  void defineMethod(ObjString* name);

  CallFrame frames[FRAMES_MAX];
  int frameCount;

  Value stack[STACK_MAX];
  Value* stackTop = nullptr;
  Table strings;

  ObjString* initString = nullptr;

  Table globals;
  ObjUpvalue* openUpValues = nullptr;

  MemoryManager* mm = nullptr;
};
