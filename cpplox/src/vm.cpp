#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "vm.h"

#include <fmt/format.h>
#include <fmt/printf.h>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "objstring.h"
#include "parser.h"
#include "table.h"
#include "value.h"

namespace
{
Value clockNative(int, Value*)
{
  auto now = std::chrono::system_clock::now().time_since_epoch();
  auto time = static_cast<double>(
      std::chrono::duration_cast<std::chrono::seconds>(now).count());

  return Value(time);
}

bool isFalsey(Value value)
{
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

}  // namespace

VM::VM()
{
  mm = new MemoryManager();
  mm->setVm(this);

  resetStack();

  initString = nullptr;
  initString = mm->copyString(std::string {"init"});

  defineNative("clock", clockNative);
}

VM::~VM()
{
  delete mm;
  mm = nullptr;

  initString = nullptr;
}

void VM::resetStack()
{
  stackTop = stack;
  frameCount = 0;
  openUpValues = nullptr;
}

void VM::runtimeError(std::string msg)
{
  std::cerr << msg << "\n";

  for (int i = frameCount - 1; i >= 0; i--) {
    auto* frame = &frames[i];
    auto* function = frame->closure->function();
    size_t instruction = frame->ip - function->chunk()->codeBegin() - 1;
    std::cerr << fmt::sprintf("[line %d] in ",
                              function->chunk()->linesAt(instruction));
    if (function->name() == nullptr) {
      std::cerr << fmt::sprintf("script\n");
    } else {
      std::cerr << fmt::sprintf("%s()\n", function->name()->string());
    }
  }

  resetStack();
}

void VM::defineNative(std::string name, NativeFn function)
{
  push(Value(mm->copyString(name)));
  push(Value(mm->newNative(function)));
  globals.set(AS_STRING(stack[0]), stack[1]);
  pop();
  pop();
}

Value VM::peek(int distance)
{
  return stackTop[-1 - distance];
}

bool VM::call(ObjClosure* closure, int argCount)
{
  if (argCount != closure->function()->arity()) {
    runtimeError(fmt::sprintf("Expected %d arguments but got %d.",
                              closure->function()->arity(),
                              argCount));
    return false;
  }

  if (frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  auto* frame = &frames[frameCount++];
  frame->closure = closure;
  frame->ip = closure->function()->chunk()->codeBegin();

  // -1 for stack slot 0, which is needed for methods
  frame->slots = stackTop - argCount - 1;

  return true;
}

bool VM::callValue(Value callee, int argCount)
{
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case ObjType::NATIVE: {
        auto native = AS_NATIVE(callee);
        auto result = native(argCount, stackTop - argCount);
        stackTop -= argCount + 1;
        push(result);
        return true;
      }
      case ObjType::CLOSURE: {
        return call(AS_CLOSURE(callee), argCount);
      }

      case ObjType::CLASS: {
        ObjClass* klass = AS_CLASS(callee);
        stackTop[-argCount - 1] = Value(mm->newInstance(klass));

        auto initializer = klass->methods()->get(initString);

        if (initializer.has_value()) {
          return call(AS_CLOSURE(initializer.value()), argCount);
        } else if (argCount != 0) {
          runtimeError(
              fmt::sprintf("Expected 0 arguments but got %d.", argCount));
          return false;
        }

        return true;
      }

      case ObjType::BOUND_METHOD: {
        ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
        stackTop[-argCount - 1] = bound->receiver();
        return call(bound->method(), argCount);
      }

      case ObjType::FUNCTION:  // fallthrough
      case ObjType::STRING:  // fallthrough
      case ObjType::UPVALUE:  // fallthrough
      case ObjType::INSTANCE:  // fallthrough
        break;
    }
  }

  runtimeError("Can only call functions and classes.");
  return false;
}

bool VM::invokeFromClass(ObjClass* klass, ObjString* name, int argCount)
{
  auto method = klass->methods()->get(name);
  if (!method.has_value()) {
    runtimeError(fmt::sprintf("Undefined property '%s'.", name->string()));
    return false;
  }

  return call(AS_CLOSURE(method.value()), argCount);
}

bool VM::invoke(ObjString* name, int argCount)
{
  Value receiver = peek(argCount);

  if (!IS_INSTANCE(receiver)) {
    runtimeError("Only instances have methods.");
    return false;
  }

  ObjInstance* instance = AS_INSTANCE(receiver);

  auto value = instance->fields()->get(name);
  if (value.has_value()) {
    stackTop[-argCount - 1] = value.value();
    return callValue(value.value(), argCount);
  }

  return invokeFromClass(instance->klass(), name, argCount);
}

bool VM::bindMethod(ObjClass* klass, ObjString* name)
{
  auto method = klass->methods()->get(name);
  if (!method.has_value()) {
    runtimeError(fmt::sprintf("Undefined property '%s'.", name->string()));
    return false;
  }

  ObjBoundMethod* bound =
      mm->newBoundMethod(peek(0), AS_CLOSURE(method.value()));
  pop();
  push(Value(bound));
  return true;
}

ObjUpvalue* VM::captureUpvalue(Value* local)
{
  ObjUpvalue* prevUpvalue = nullptr;
  ObjUpvalue* upvalue = openUpValues;

  while (upvalue != nullptr && upvalue->location() > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->nextUpvalue();
  }

  if (upvalue != nullptr && upvalue->location() == local) {
    return upvalue;
  }

  ObjUpvalue* createdUpvalue = mm->newUpvalue(local);
  createdUpvalue->setNextUpvalue(upvalue);

  if (prevUpvalue == nullptr) {
    openUpValues = createdUpvalue;
  } else {
    prevUpvalue->setNextUpvalue(createdUpvalue);
  }

  return createdUpvalue;
}

void VM::closeUpvalues(Value* last)
{
  while (openUpValues != nullptr && openUpValues->location() >= last) {
    ObjUpvalue* upvalue = openUpValues;
    upvalue->setClosed(*upvalue->location());
    upvalue->setLocation(upvalue->closed());
    openUpValues = upvalue->nextUpvalue();
  }
}

void VM::defineMethod(ObjString* name)
{
  Value method = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));
  klass->methods()->set(name, method);
  pop();
}

void VM::concatenate()
{
  assert(IS_STRING(peek(0)));
  assert(IS_STRING(peek(1)));

  ObjString* b = AS_STRING(peek(0));
  ObjString* a = AS_STRING(peek(1));

  assert(b != nullptr);
  assert(a != nullptr);

  ObjString* result = mm->takeString(a->string() + b->string());
  pop();
  pop();
  push(Value(result));
}

void VM::push(Value value)
{
  *stackTop = value;
  stackTop++;
}

Value VM::pop()
{
  stackTop--;
  return *stackTop;
}

InterpretResult VM::run()
{
  CallFrame* frame = &frames[frameCount - 1];

  const auto READ_BYTE = [](auto* f) -> uint8_t { return (*f->ip++); };
  const auto READ_SHORT = [](auto* f) -> uint16_t {
    f->ip += 2;
    return static_cast<uint16_t>((f->ip[-2] << 8) | f->ip[-1]);
  };

  const auto READ_CONSTANT = [&]() -> Value {
    return (frame->closure->function()->chunk()->constantsAt(READ_BYTE(frame)));
  };

#define BINARY_OP(valueType, op) \
  do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      runtimeError("Operands must be numbers."); \
      return InterpretResult::RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
  } while (false)

  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    std::cout << "          ";
    for (Value* slot = stack; slot < stackTop; slot++) {
      std::cout << "[ ";
      std::cout << toString(*slot);
      std::cout << " ]";
    }
    std::cout << "\n";
    disassembleInstruction(
        frame->closure->function()->chunk(),
        (frame->ip - frame->closure->function()->chunk()->codeBegin()));

#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE(frame)) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }

      case OP_RETURN: {
        Value result = pop();
        closeUpvalues(frame->slots);
        frameCount--;
        if (frameCount == 0) {
          pop();
          return InterpretResult::OK;
        }

        stackTop = frame->slots;
        push(result);
        frame = &frames[frameCount - 1];
        break;
      }

      case OP_NEGATE: {
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be a number.");
          return InterpretResult::RUNTIME_ERROR;
        }

        push(Value(-(AS_NUMBER(pop()))));
        break;
      }

      case OP_EQUAL: {
        const Value b = pop();
        const Value a = pop();
        push(Value(valuesEqual(a, b)));
        break;
      }

      case OP_GREATER:
        BINARY_OP(Value, >);
        break;

      case OP_LESS:
        BINARY_OP(Value, <);
        break;

      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(Value(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUBTRACT:
        BINARY_OP(Value, -);
        break;

      case OP_MULTIPLY:
        BINARY_OP(Value, *);
        break;

      case OP_DIVIDE:
        BINARY_OP(Value, /);
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
        std::cout << toString(pop()) << "\n";
        break;
      }

      case OP_POP:
        pop();
        break;

      case OP_DEFINE_GLOBAL: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        globals.set(name, peek(0));
        pop();
        break;
      }

      case OP_GET_GLOBAL: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        auto value = globals.get(name);
        if (!value.has_value()) {
          runtimeError(
              fmt::sprintf("Undefined variable '%s'.", name->string()));
          return InterpretResult::RUNTIME_ERROR;
        }

        push(value.value());
        break;
      }

      case OP_SET_GLOBAL: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        if (globals.set(name, peek(0))) {
          globals.remove(name);
          runtimeError(
              fmt::sprintf("Undefined variable '%s'.", name->string()));
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE(frame);
        push(frame->slots[slot]);
        break;
      }

      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE(frame);
        frame->slots[slot] = peek(0);
        break;
      }

      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT(frame);
        if (isFalsey(peek(0))) {
          frame->ip += offset;
        }
        break;
      }

      case OP_JUMP: {
        uint16_t offset = READ_SHORT(frame);
        frame->ip += offset;
        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT(frame);
        frame->ip -= offset;
        break;
      }

      case OP_CALL: {
        int argCount = READ_BYTE(frame);
        if (!callValue(peek(argCount), argCount)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        frame = &frames[frameCount - 1];
        break;
      }

      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = mm->newClosure(function);
        push(Value(closure));

        for (int i = 0; i < closure->upvalueCount(); i++) {
          uint8_t isLocal = READ_BYTE(frame);
          uint8_t index = READ_BYTE(frame);
          if (isLocal) {
            closure->setUpvalue(captureUpvalue(frame->slots + index), i);
          } else {
            closure->setUpvalue(frame->closure->upvalue(index), i);
          }
        }

        break;
      }

      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE(frame);
        push(*frame->closure->upvalue(slot)->location());
        break;
      }

      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE(frame);
        *frame->closure->upvalue(slot)->location() = peek(0);
        break;
      }

      case OP_CLOSE_UPVALUE: {
        closeUpvalues(stackTop - 1);
        pop();
        break;
      }

      case OP_CLASS: {
        push(Value(mm->newClass(AS_STRING(READ_CONSTANT()))));
        break;
      }

      case OP_GET_PROPERTY: {
        if (!IS_INSTANCE(peek(0))) {
          runtimeError("Only instances have properties.");
          return InterpretResult::RUNTIME_ERROR;
        }

        ObjInstance* instance = AS_INSTANCE(peek(0));
        ObjString* name = AS_STRING(READ_CONSTANT());

        auto value = instance->fields()->get(name);
        if (value.has_value()) {
          pop();  // pop instance
          push(value.value());
          break;
        }

        if (!bindMethod(instance->klass(), name)) {
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
        instance->fields()->set(AS_STRING(READ_CONSTANT()), peek(0));
        Value value = pop();
        pop();
        push(value);
        break;
      }

      case OP_METHOD: {
        defineMethod(AS_STRING(READ_CONSTANT()));
        break;
      }

      case OP_INVOKE: {
        ObjString* method = AS_STRING(READ_CONSTANT());
        int argCount = READ_BYTE(frame);
        if (!invoke(method, argCount)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        frame = &frames[frameCount - 1];
        break;
      }

      case OP_INHERIT: {
        Value superclass = peek(1);

        if (!IS_CLASS(superclass)) {
          runtimeError("Superclass must be a class.");
          return InterpretResult::RUNTIME_ERROR;
        }

        ObjClass* subclass = AS_CLASS(peek(0));
        subclass->methods()->addAll(AS_CLASS(superclass)->methods());
        pop();  // subclass
        break;
      }

      case OP_GET_SUPER: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        ObjClass* superclass = AS_CLASS(pop());

        if (!bindMethod(superclass, name)) {
          return InterpretResult::RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUPER_INVOKE: {
        ObjString* method = AS_STRING(READ_CONSTANT());
        int argCount = READ_BYTE(frame);
        ObjClass* superclass = AS_CLASS(pop());
        if (!invokeFromClass(superclass, method, argCount)) {
          return InterpretResult::RUNTIME_ERROR;
        }

        frame = &frames[frameCount - 1];
        break;
      }
    }
  }

#undef BINARY_OP
}

InterpretResult VM::interpret(std::string_view source)
{
  Scanner scanner {source};
  Parser parser {scanner};

  Compiler compiler {nullptr, mm, parser, FunctionType::SCRIPT};
  auto* function = compiler.compile();
  if (function == nullptr) {
    return InterpretResult::COMPILE_ERROR;
  }

  push(Value(function));
  ObjClosure* closure = mm->newClosure(function);
  pop();
  push(Value(closure));
  call(closure, 0);  // initialize "function" which houses top level code

  return run();
}