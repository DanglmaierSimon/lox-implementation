#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include "object.h"
#include "vm.h"

void freeObjects();
void markValue(Value value);
void markObject(Obj* object);
void collectGarbage();

constexpr size_t GROW_CAPACITY(size_t capacity)
{
  return (capacity < 8) ? 8 : (capacity * 2);
}

template<typename T>
inline T* reallocate(auto pointer, size_t oldSize, size_t newSize)
{
  vm.bytesAllocated += newSize - oldSize;

  if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
    collectGarbage();
#else
    if (vm.bytesAllocated > vm.nextGC) {
      collectGarbage();
    }
#endif
  }

  if (newSize == 0) {
    free(pointer);
    return nullptr;
  }

  T* result = (T*)realloc(pointer, newSize);

  if (result == nullptr) {
    exit(1);
  }
  return result;
}

template<typename T>
T* ALLOCATE(auto count)
{
  return reallocate<T>(nullptr, 0, sizeof(T) * (count));
}

template<typename T>
T* GROW_ARRAY(T* pointer, auto oldCount, auto newCount)
{
  return reallocate<T>(pointer, sizeof(T) * (oldCount), sizeof(T) * (newCount));
}

template<typename T>
void FREE(auto* pointer)
{
  reallocate<T>(pointer, sizeof(T), 0);
}

template<typename T>
inline void FREE_ARRAY(T* pointer, auto oldCount)
{
  reallocate<T>(pointer, sizeof(T) * (oldCount), 0);
}
