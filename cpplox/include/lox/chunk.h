#pragma once

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
  OP_DEFINE_GLOBAL_VAR,
  OP_DEFINE_GLOBAL_CONST,
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

class Chunk
{
public:
  // code
  size_t count() const;
  void write(uint8_t byte, size_t line);
  void writeAt(size_t idx, uint8_t byte);
  uint8_t codeAt(size_t idx) const;
  const uint8_t* codeBegin() const;

  // constants
  size_t addConstant(Value value);
  std::vector<Value> constants() const;
  Value constantsAt(size_t idx) const;

  // lines
  size_t linesAt(size_t idx) const;

private:
  std::vector<uint8_t> _code;
  std::vector<Value> _constants;
  std::vector<size_t> _lines;
};
