#include <algorithm>
#include <vector>

#include "lox/memory.h"

#include "lox/chunk.h"
#include "lox/compiler.h"
#include "lox/table.h"
#include "lox/value.h"
#include "lox/vm.h"

#ifdef DEBUG_LOG_GC
#  include "lox/debug.h"
#endif

auto constexpr GC_HEAP_GROW_FACTOR = 2;

static void freeObject(Obj* object, MemoryManager* mm)
{
  assert(object != nullptr);

#ifdef DEBUG_LOG_GC
  std::cout << fmt::sprintf(
      "%p free type %d\n", (void*)object, static_cast<int>(object->type()));
#endif

  switch (object->type()) {
    case ObjType::STRING: {
      ObjString* string = (ObjString*)object;
      mm->FREE<ObjString>(string);
      break;
    }

    case ObjType::FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      mm->FREE<ObjFunction>(function);
      break;
    }

    case ObjType::NATIVE: {
      mm->FREE<ObjNative>((ObjNative*)object);
      break;
    }

    case ObjType::CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      mm->FREE<ObjClosure>(closure);
      break;
    }

    case ObjType::UPVALUE: {
      mm->FREE<ObjUpvalue>((ObjUpvalue*)object);
      break;
    }

    case ObjType::CLASS: {
      ObjClass* klass = (ObjClass*)object;
      mm->FREE<ObjClass>(klass);
      break;
    }

    case ObjType::INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      mm->FREE<ObjInstance>(instance);
      break;
    }

    case ObjType::BOUND_METHOD: {
      mm->FREE<ObjBoundMethod>((ObjBoundMethod*)object);
      break;
    }
  }
}

void MemoryManager::freeObjects(Obj* objs)
{
  Obj* object = objs;
  while (object != nullptr) {
    Obj* next = object->nextObj();
    freeObject(object, this);
    object = next;
  }
}

void MemoryManager::markRoots()
{
  for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
    markValue(*slot);
  }

  for (int i = 0; i < vm->frameCount; i++) {
    markObject((Obj*)vm->frames[i].closure);
  }

  for (ObjUpvalue* upvalue = vm->openUpValues; upvalue != nullptr;
       upvalue = upvalue->nextUpvalue())
  {
    markObject((Obj*)(upvalue));
  }

  vm->globals.mark(this);
  markCompilerRoots();
  markObject((Obj*)vm->initString);
}

void MemoryManager::markArray(const std::vector<Value>& array)
{
  for (const auto& value : array) {
    markValue(value);
  }
}

void MemoryManager::blackenObject(Obj* object)
{
  assert(object != nullptr);

#ifdef DEBUG_LOG_GC
  std::cout << fmt::sprintf("%p blacken ", (void*)object);
  printValue(Value(object));
  std::cout << "\n";
#endif

  switch (object->type()) {
    case ObjType::UPVALUE: {
      auto obj = ((ObjUpvalue*)object);
      markValue(*obj->closed());
      break;
    }
    case ObjType::CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      markObject((Obj*)closure->function());
      for (int i = 0; i < closure->upvalueCount(); i++) {
        markObject((Obj*)closure->upvalue(i));
      }
      break;
    }

    case ObjType::FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      markObject((Obj*)function->name());
      markArray(function->chunk()->constants());
      break;
    }

    case ObjType::CLASS: {
      ObjClass* klass = (ObjClass*)object;
      markObject(klass->name());
      klass->methods()->mark(this);
      break;
    }

    case ObjType::INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      markObject(instance->klass());
      instance->fields()->mark(this);
      break;
    }

    case ObjType::BOUND_METHOD: {
      ObjBoundMethod* bound = (ObjBoundMethod*)object;
      markValue(bound->receiver());
      markObject(bound->method());
      break;
    }

    case ObjType::NATIVE:  // fallthrough
    case ObjType::STRING:  // fallthrough
      break;
  }
}

void MemoryManager::traceReferences()
{
  while (grayStack.size() > 0) {
    Obj* object = grayStack.back();
    grayStack.pop_back();
    blackenObject(object);
  }
}

void MemoryManager::sweep()
{
  Obj* previous = nullptr;
  Obj* object = objects;

  while (object != nullptr) {
    if (object->isMarked()) {
      object->setIsMarked(false);
      previous = object;
      object = object->nextObj();
    } else {
      Obj* unreached = object;
      object = object->nextObj();

      if (previous != nullptr) {
        previous->setNextObj(object);
      } else {
        objects = object;
      }

      freeObject(unreached, this);
    }
  }
}

void MemoryManager::collectGarbage()
{
#ifdef DEBUG_LOG_GC
  std::cout << "DBG: -- gc begin\n";
  size_t before = bytesAllocated;
#endif

  markRoots();
  traceReferences();
  vm->strings.removeWhite();
  sweep();

  nextGC = bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
  std::cout << "DBG: -- gc end\n";
  std::cout << fmt::sprintf(
      "DBG:   collected %zu bytes (from %zu to %zu) next at %zu\n",
      before - bytesAllocated,
      before,
      bytesAllocated,
      nextGC);
#endif
}

void MemoryManager::markValue(Value value)
{
  if (IS_OBJ(value)) {
    markObject(AS_OBJ(value));
  }
}

void MemoryManager::markObject(Obj* object)
{
  if (object == nullptr || object->isMarked()) {
    return;
  }

#ifdef DEBUG_LOG_GC
  std::cout << fmt::sprintf("DBG: %p mark ", (void*)object);
  printValue(Value(object));
  std::cout << "\n";
#endif

  object->setIsMarked(true);

  grayStack.push_back(object);
}

ObjBoundMethod* MemoryManager::newBoundMethod(Value receiver,
                                              ObjClosure* method)
{
  return ALLOCATE_OBJ<ObjBoundMethod>(receiver, method);
}

ObjInstance* MemoryManager::newInstance(ObjClass* klass)
{
  return ALLOCATE_OBJ<ObjInstance>(klass);
}

ObjClass* MemoryManager::newClass(ObjString* name)
{
  return ALLOCATE_OBJ<ObjClass>(name);
}

ObjUpvalue* MemoryManager::newUpvalue(Value* slot)
{
  return ALLOCATE_OBJ<ObjUpvalue>(slot, nullptr, Value {});
}

ObjClosure* MemoryManager::newClosure(ObjFunction* function)
{
  return ALLOCATE_OBJ<ObjClosure>(
      function, std::vector<ObjUpvalue*>(function->upvalueCount()));
}

ObjFunction* MemoryManager::newFunction()
{
  return ALLOCATE_OBJ<ObjFunction>(0, 0, nullptr);
}

ObjNative* MemoryManager::newNative(NativeFn function)
{
  return ALLOCATE_OBJ<ObjNative>(function);
}

Compiler* MemoryManager::currentCompiler()
{
  return _currentCompiler;
}

void MemoryManager::setCurrentCompiler(Compiler* compiler)
{
  _currentCompiler = compiler;
}

void MemoryManager::markCompilerRoots()
{
  Compiler* compiler = currentCompiler();
  while (compiler != nullptr) {
    compiler->mm->markObject(compiler->function);
    compiler = compiler->enclosing;
  }
}
