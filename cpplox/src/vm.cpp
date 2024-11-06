#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ostream>

#include "vm.h"

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
#include "table.h"
#include "value.h"

VM vm;

static Value clockNative(int, Value*)
{
  return Value((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack()
{
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpValues = nullptr;
}

static void runtimeError(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    auto* frame = &vm.frames[i];
    auto* function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (function->name == nullptr) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  resetStack();
}

static void defineNative(const char* name, NativeFn function)
{
  push(Value(copyString(name, (int)strlen(name))));
  push(Value(newNative(function)));
  vm.globals.set(AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static Value peek(int distance)
{
  return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount)
{
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.",
                 closure->function->arity,
                 argCount);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  auto* frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;

  // -1 for stack slot 0, which is needed for methods
  frame->slots = vm.stackTop - argCount - 1;

  return true;
}

static bool callValue(Value callee, int argCount)
{
  if ((callee).is_obj()) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_NATIVE: {
        auto native = AS_NATIVE(callee);
        auto result = native(argCount, vm.stackTop - argCount);
        vm.stackTop -= argCount + 1;
        push(result);
        return true;
      }
      case OBJ_CLOSURE: {
        return call(AS_CLOSURE(callee), argCount);
      }

      case OBJ_CLASS: {
        ObjClass* klass = AS_CLASS(callee);
        vm.stackTop[-argCount - 1] = Value(newInstance(klass));

        if (auto initializer = klass->methods.get(vm.initString);
            initializer.has_value())
        {
          return call(AS_CLOSURE(*initializer), argCount);
        } else if (argCount != 0) {
          runtimeError("Expected 0 arguments but got %d.", argCount);
          return false;
        }

        return true;
      }

      case OBJ_BOUND_METHOD: {
        ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
        vm.stackTop[-argCount - 1] = bound->receiver;
        return call(bound->method, argCount);
      }

      case OBJ_FUNCTION:  // fallthrough
      case OBJ_STRING:  // fallthrough
      case OBJ_UPVALUE:  // fallthrough
      case OBJ_INSTANCE:  // fallthrough
        break;
    }
  }

  runtimeError("Can only call functions and classes.");
  return false;
}

static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount)
{
  auto method = klass->methods.get(name);
  if (!method.has_value()) {
    runtimeError("Undefined property '%s'.", name->chars);
    return false;
  }

  return call(AS_CLOSURE(*method), argCount);
}

static bool invoke(ObjString* name, int argCount)
{
  Value receiver = peek(argCount);

  if (!IS_INSTANCE(receiver)) {
    runtimeError("Only instances have methods.");
    return false;
  }

  ObjInstance* instance = AS_INSTANCE(receiver);

  auto value = instance->fields.get(name);
  if (value) {
    vm.stackTop[-argCount - 1] = *value;
    return callValue(*value, argCount);
  }

  return invokeFromClass(instance->klass, name, argCount);
}

static bool bindMethod(ObjClass* klass, ObjString* name)
{
  auto method = klass->methods.get(name);
  if (!method.has_value()) {
    runtimeError("Undefined property '%s'.", name->chars);
    return false;
  }

  ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(*method));
  pop();
  push(Value(bound));
  return true;
}

static ObjUpvalue* captureUpvalue(Value* local)
{
  ObjUpvalue* prevUpvalue = nullptr;
  ObjUpvalue* upvalue = vm.openUpValues;

  while (upvalue != nullptr && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->nextupval;
  }

  if (upvalue != nullptr && upvalue->location == local) {
    return upvalue;
  }

  ObjUpvalue* createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;

  if (prevUpvalue == nullptr) {
    vm.openUpValues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }

  return createdUpvalue;
}

static void closeUpvalues(Value* last)
{
  while (vm.openUpValues != nullptr && vm.openUpValues->location >= last) {
    ObjUpvalue* upvalue = vm.openUpValues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpValues = upvalue->nextupval;
  }
}

static void defineMethod(ObjString* name)
{
  Value method = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));
  klass->methods.set(name, method);
  pop();
}

constexpr bool isFalsey(const Value& value)
{
  return value.is_nil() || (value.is_bool() && !value.as_bool());
}

static void concatenate()
{
  assert(IS_STRING(peek(0)));
  assert(IS_STRING(peek(1)));

  ObjString* b = AS_STRING(peek(0));
  ObjString* a = AS_STRING(peek(1));

  assert(b != nullptr);
  assert(a != nullptr);

  int len = a->length + b->length;
  char* chars = ALLOCATE<char>(len + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[len] = '\0';

  ObjString* result = takeString(chars, len);
  pop();
  pop();
  push(Value(result));
}

void initVM()
{
  vm.compiler.reset();
  resetStack();
  vm.objects = nullptr;

  vm.grayCapacity = 0;
  vm.grayCount = 0;
  vm.grayStack = nullptr;

  vm.bytesAllocated = 0;
  vm.nextGC = 1048576;  // 1024*1024

  vm.initString = nullptr;
  vm.initString = copyString("init", 4);

  defineNative("clock", clockNative);
}

void freeVM()
{
  freeObjects();
  vm.initString = nullptr;
  free(vm.grayStack);
  vm.compiler.reset();
}

void push(Value value)
{
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop()
{
  vm.stackTop--;
  return *vm.stackTop;
}

static InterpretResult run()
{
  CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
  (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(op) \
  do { \
    if (!(peek(0).is_number()) || !(peek(1).is_number())) { \
      runtimeError("Operands must be numbers."); \
      return InterpretResult::RUNTIME_ERROR; \
    } \
    double b = (pop().as_number()); \
    double a = (pop().as_number()); \
    push(Value(a op b)); \
  } while (false)

  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(
        &frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }

      case OP_RETURN: {
        Value result = pop();
        closeUpvalues(frame->slots);
        vm.frameCount--;
        if (vm.frameCount == 0) {
          pop();
          return InterpretResult::OK;
        }

        vm.stackTop = frame->slots;
        push(result);
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_NEGATE: {
        if (!(peek(0)).is_number()) {
          runtimeError("Operand must be a number.");
          return InterpretResult::RUNTIME_ERROR;
        }

        push(Value(-(pop().as_number())));
        break;
      }

      case OP_EQUAL: {
        const Value b = pop();
        const Value a = pop();
        push(Value(valuesEqual(a, b)));
        break;
      }

      case OP_GREATER:
        BINARY_OP(>);
        break;

      case OP_LESS:
        BINARY_OP(<);
        break;

      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if ((peek(0).is_number()) && (peek(1).is_number())) {
          double b = (pop().as_number());
          double a = (pop().as_number());
          push(Value(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUBTRACT:
        BINARY_OP(-);
        break;

      case OP_MULTIPLY:
        BINARY_OP(*);
        break;

      case OP_DIVIDE:
        BINARY_OP(/);
        break;

      case OP_NOT:
        push(Value(isFalsey(pop())));
        break;

      case OP_NIL:
        push(Value());
        break;

      case OP_TRUE:
        push(Value(true));
        break;

      case OP_FALSE:
        push(Value(false));
        break;

      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        break;
      }

      case OP_POP:
        pop();
        break;

      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        vm.globals.set(name, peek(0));
        pop();
        break;
      }

      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        auto value = vm.globals.get(name);
        if (!value.has_value()) {
          runtimeError("Undefined variable '%s'.", name->chars);
          return InterpretResult::RUNTIME_ERROR;
        }

        push(*value);
        break;
      }

      case OP_SET_GLOBAL: {
        ObjString* name = READ_STRING();
        if (vm.globals.set(name, peek(0))) {
          vm.globals.deleteKey(name);
          runtimeError("Undefined variable '%s'.", name->chars);
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }

      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
        break;
      }

      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (isFalsey(peek(0))) {
          frame->ip += offset;
        }
        break;
      }

      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        frame->ip += offset;
        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->ip -= offset;
        break;
      }

      case OP_CALL: {
        int argCount = READ_BYTE();
        if (!callValue(peek(argCount), argCount)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = newClosure(function);
        push(Value(closure));

        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }

        break;
      }

      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }

      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
        break;
      }

      case OP_CLOSE_UPVALUE: {
        closeUpvalues(vm.stackTop - 1);
        pop();
        break;
      }

      case OP_CLASS: {
        push(Value(newClass(READ_STRING())));
        break;
      }

      case OP_GET_PROPERTY: {
        if (!IS_INSTANCE(peek(0))) {
          runtimeError("Only instances have properties.");
          return InterpretResult::RUNTIME_ERROR;
        }

        ObjInstance* instance = AS_INSTANCE(peek(0));
        ObjString* name = READ_STRING();

        auto value = instance->fields.get(name);
        if (value.has_value()) {
          pop();  // pop instance
          push(*value);
          break;
        }

        if (!bindMethod(instance->klass, name)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_SET_PROPERTY: {
        if (!IS_INSTANCE(peek(1))) {
          runtimeError("Only instances have fields.");
          return InterpretResult::RUNTIME_ERROR;
        }

        ObjInstance* instance = AS_INSTANCE(peek(1));
        instance->fields.set(READ_STRING(), peek(0));
        Value value = pop();
        pop();
        push(value);
        break;
      }

      case OP_METHOD: {
        defineMethod(READ_STRING());
        break;
      }

      case OP_INVOKE: {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();
        if (!invoke(method, argCount)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_INHERIT: {
        Value superclass = peek(1);

        if (!IS_CLASS(superclass)) {
          runtimeError("Superclass must be a class.");
          return InterpretResult::RUNTIME_ERROR;
        }

        ObjClass* subclass = AS_CLASS(peek(0));
        subclass->methods.addAll(AS_CLASS(superclass)->methods);
        pop();  // subclass
        break;
      }

      case OP_GET_SUPER: {
        ObjString* name = READ_STRING();
        ObjClass* superclass = AS_CLASS(pop());

        if (!bindMethod(superclass, name)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUPER_INVOKE: {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();
        ObjClass* superclass = AS_CLASS(pop());
        if (!invokeFromClass(superclass, method, argCount)) {
          return InterpretResult::RUNTIME_ERROR;
        }

        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      default:
        std::cout << "Unknown instruction: " << instruction << std::endl;
        std::abort();
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source)
{
  auto scanner = std::make_shared<Scanner>(source);
  auto parser = std::make_shared<Parser>(scanner);

  auto compiler = std::make_shared<Compiler>(parser, TYPE_SCRIPT);

  vm.compiler = compiler;

  auto* function = compiler->compile();
  if (function == nullptr) {
    return InterpretResult::COMPILER_ERROR;
  }

  push(Value(function));
  ObjClosure* closure = newClosure(function);
  pop();
  push(Value(closure));
  call(closure, 0);  // initialize "function" which houses top level code

  return run();
}