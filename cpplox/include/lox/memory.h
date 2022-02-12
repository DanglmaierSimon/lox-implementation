#pragma once

#include <iostream>
#include <variant>

#include <fmt/printf.h>

#include "common.h"
#include "lox/objects/obj.h"
#include "lox/objects/objboundmethod.h"
#include "lox/objects/objclass.h"
#include "lox/objects/objclosure.h"
#include "lox/objects/objfunction.h"
#include "lox/objects/objinstance.h"
#include "lox/objects/objnative.h"
#include "lox/objects/objstring.h"
#include "lox/objects/objupvalue.h"
#include "vm.h"

class Compiler;
// TODO: Clean up this whole mess!! Jesus

template<typename T>
constexpr auto GROW_CAPACITY(T capacity)
{
  return ((capacity) < 8 ? 8 : (capacity)*2);
}

class MemoryManager
{
public:
  virtual ~MemoryManager()
  {
    freeObjects(objects);
  }

  template<typename T>
  inline void FREE(T* pointer)
  {
    bytesAllocated = bytesAllocated - sizeof(T);
    delete pointer;
  }

  template<typename T, typename... Args>
  inline T* ALLOCATE_OBJ(Args... args)
  {
    auto size = sizeof(T);

    bytesAllocated = bytesAllocated + size;

    maybeGC();

    T* object = new T(std::forward<Args>(args)...);
    object->setNextObj(objects);
    objects = object;

#ifdef DEBUG_LOG_GC
    std::cout << fmt::sprintf("%p allocate %zu for %d\n",
                              (void*)object,
                              size,
                              static_cast<int>(object->type()));
#endif

    return object;
  }

  inline void maybeGC()
  {
    if (bytesAllocated > nextGC) {
      collectGarbage();
    }
  }

  inline ObjString* allocateString(std::string chars, uint32_t hash)
  {
    ObjString* string = ALLOCATE_OBJ<ObjString>(chars, hash);

    vm->strings.set(string, Value {});

    return string;
  }

  constexpr uint32_t hashString(const char* key, size_t length)
  {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
      hash ^= (uint8_t)key[i];
      hash *= 16777619;
    }

    return hash;
  }

  uint32_t hashString(std::string string)
  {
    return hashString(string.c_str(), string.length());
  }

  inline ObjString* takeString(std::string string)
  {
    uint32_t hash = hashString(string);

    ObjString* interned = vm->strings.findString(string, hash);
    if (interned != nullptr) {
      return interned;
    }

    return allocateString(string, hash);
  }

  inline ObjString* copyString(std::string chars)
  {
    uint32_t hash = hashString(chars);

    ObjString* interned = vm->strings.findString(chars, hash);
    if (interned != nullptr) {
      return interned;
    }

    return allocateString(chars, hash);
  }

  inline ObjString* copyString(std::string_view chars)
  {
    return copyString(std::string {chars});
  }

  void freeObjects(Obj* objects);
  void markValue(Value value);
  void markObject(Obj* object);
  void markCompilerRoots();
  void collectGarbage();

  inline void setVm(VM* _vm)
  {
    vm = _vm;
  }

  ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
  ObjInstance* newInstance(ObjClass* klass);
  ObjClass* newClass(ObjString* name);
  ObjUpvalue* newUpvalue(Value* slot);
  ObjClosure* newClosure(ObjFunction* function);
  ObjFunction* newFunction();
  ObjNative* newNative(NativeFn function);

  void setCurrentCompiler(Compiler* compiler);
  Compiler* currentCompiler();

private:
  void markRoots();
  void markArray(const std::vector<Value>& array);
  void blackenObject(Obj* object);
  void traceReferences();
  void sweep();

private:
  size_t bytesAllocated;
  static inline size_t nextGC = 1048576;  // 1024*1024
  Obj* objects;

  std::vector<Obj*> grayStack;

  VM* vm = nullptr;
  Compiler* _currentCompiler = nullptr;
};