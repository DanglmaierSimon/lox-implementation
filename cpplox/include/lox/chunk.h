#pragma once

#include <cstdint>
#include <vector>

#include "common.h"
#include "value.h"

enum OpCode : uint8_t
{
  OP_CONSTANT,

  // Literal values
  OP_NIL,
  OP_TRUE,
  OP_FALSE,

  // Unary Operators
  OP_NEGATE,
  OP_NOT,

  // Binary operators
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,

  // Comparison operators
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,

  // Return
  OP_RETURN,

  // Built-in funtions
  OP_PRINT,

  OP_POP,
  OP_DEFINE_GLOBAL,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_JUMP_IF_FALSE,
  OP_JUMP,
  OP_LOOP,
  OP_CALL,
  OP_CLOSURE,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_CLOSE_UPVALUE,
  OP_CLASS,
  OP_SET_PROPERTY,
  OP_GET_PROPERTY,
  OP_METHOD,
  OP_INVOKE,
  OP_INHERIT,
  OP_GET_SUPER,
  OP_SUPER_INVOKE,
};

struct Chunk
{
  Chunk() = default;
  virtual ~Chunk() = default;

  inline int count() const
  {
    return static_cast<int>(code.size());
  }

  void write(uint8_t byte, int line)
  {
    code.push_back(byte);
    lines.push_back(line);
  }

  int addConstant(Value value)
  {
    constants.push_back(value);
    return constants.size() - 1;
  }

  std::vector<uint8_t> code;
  std::vector<int> lines;
  std::vector<Value> constants;
};

void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
