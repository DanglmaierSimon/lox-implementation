#include <iostream>
#include <string>
#include <vector>

#include "lox/debug.h"

#include <fmt/printf.h>

#include "lox/chunk.h"
#include "lox/objects/objfunction.h"
#include "lox/value.h"

namespace
{
size_t simpleInstruction(const char* name, size_t offset)
{
  std::cout << name << "\n";
  return offset + 1;
}

size_t byteInstruction(const char* name, Chunk* chunk, size_t offset)
{
  uint8_t slot = chunk->codeAt(offset + 1);
  std::cout << fmt::sprintf("%-16s %4d\n", name, slot);
  return offset + 2;
}

size_t constantInstruction(const char* name, Chunk* chunk, size_t offset)
{
  const auto constant = chunk->codeAt(offset + 1);
  std::cout << fmt::sprintf("%-16s %4d '", name, constant);
  std::cout << toString(chunk->constantsAt(constant));
  std::cout << std::endl;
  return offset + 2;
}

size_t jumpInstruction(const char* name,
                       size_t sign,
                       Chunk* chunk,
                       size_t offset)
{
  auto jump = (uint16_t)(chunk->codeAt(offset + 1) << 8);
  jump |= chunk->codeAt(offset + 2);

  std::cout << fmt::sprintf(
      "%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

size_t invokeInstruction(const char* name, Chunk* chunk, size_t offset)
{
  uint8_t constant = chunk->codeAt(offset + 1);
  uint8_t argCount = chunk->codeAt(offset + 2);
  std::cout << fmt::sprintf("%-16s (%d args) %4d '", name, argCount, constant);
  std::cout << toString(chunk->constantsAt(constant));
  std::cout << fmt::sprintf("'\n");
  return offset + 3;
}

}  // namespace

void disassembleChunk(Chunk* chunk, std::string_view name)
{
  std::cout << fmt::sprintf("== %s ==\n", name);

  for (size_t offset = 0; offset < chunk->count();) {
    offset = disassembleInstruction(chunk, offset);
  }
}

size_t disassembleInstruction(Chunk* chunk, size_t offset)
{
  std::cout << fmt::sprintf("%04d ", offset);

  if (offset > 0 && chunk->linesAt(offset) == chunk->linesAt(offset - 1)) {
    std::cout << "   | ";
  } else {
    std::cout << fmt::sprintf("%4d ", chunk->linesAt(offset));
  }

  auto instruction = chunk->codeAt(offset);
  switch (instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case OP_NIL:
      return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case OP_NOT:
      return simpleInstruction("OP_NOT", offset);
    case OP_EQUAL:
      return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
      return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
      return simpleInstruction("OP_LESS", offset);
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    case OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OP_DEFINE_GLOBAL_VAR:
      return constantInstruction("OP_DEFINE_GLOBAL_VAR", chunk, offset);
    case OP_DEFINE_GLOBAL_CONST:
      return constantInstruction("OP_DEFINE_GLOBAL_CONST", chunk, offset);
    case OP_GET_GLOBAL:
      return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
      return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_JUMP:
      return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
      return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_LOOP:
      return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OP_CALL:
      return byteInstruction("OP_CALL", chunk, offset);
    case OP_GET_UPVALUE:
      return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case OP_SET_UPVALUE:
      return byteInstruction("OP_SET_UPVALUE", chunk, offset);
    case OP_CLOSE_UPVALUE:
      return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case OP_CLASS:
      return constantInstruction("OP_CLASS", chunk, offset);
    case OP_SET_PROPERTY:
      return constantInstruction("OP_SET_PROPERTY", chunk, offset);
    case OP_GET_PROPERTY:
      return constantInstruction("OP_GET_PROPERTY", chunk, offset);
    case OP_METHOD:
      return constantInstruction("OP_METHOD", chunk, offset);
    case OP_INVOKE:
      return invokeInstruction("OP_INVOKE", chunk, offset);
    case OP_INHERIT:
      return simpleInstruction("OP_INHERIT", offset);
    case OP_GET_SUPER:
      return constantInstruction("OP_GET_SUPER", chunk, offset);
    case OP_SUPER_INVOKE:
      return invokeInstruction("OP_SUPER_INVOKE", chunk, offset);
    case OP_CLOSURE: {
      offset++;
      uint8_t constant = chunk->codeAt(offset++);
      std::cout << fmt::sprintf("%-16s %4d ", "OP_CLOSURE", constant);
      std::cout << toString(chunk->constantsAt(constant));
      std::cout << fmt::sprintf("\n");

      auto* function = AS_FUNCTION(chunk->constantsAt(constant));

      for (int j = 0; j < function->upvalueCount(); j++) {
        auto isLocal = chunk->codeAt(offset++);
        auto index = chunk->codeAt(offset++);
        std::cout << fmt::sprintf("%04d      |                     %s %d\n",
                                  offset - 2,
                                  isLocal ? "local" : "upvalue",
                                  index);
      }

      return offset;
    }

    default:
      std::cout << fmt::sprintf("Unknown opcode %hhu\n", instruction);
      return offset + 1;
  }
}