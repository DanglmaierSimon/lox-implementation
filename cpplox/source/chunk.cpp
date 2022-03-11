#include <cassert>
#include <vector>

#include "lox/chunk.h"

#include "lox/value.h"

size_t Chunk::count() const
{
  return _code.size();
}

void Chunk::write(uint8_t byte, size_t line)
{
  _code.push_back(byte);
  _lines.push_back(line);
}

size_t Chunk::addConstant(Value value)
{
  _constants.push_back(value);
  return _constants.size() - 1;
}

size_t Chunk::linesAt(size_t idx) const
{
  return _lines.at(idx);
}

Value Chunk::constantsAt(size_t idx) const
{
  return _constants.at(idx);
}

std::vector<Value> Chunk::constants() const
{
  return _constants;
}

void Chunk::writeAt(size_t idx, uint8_t byte)
{
  assert(count() > idx);
  _code[idx] = byte;
}

uint8_t Chunk::codeAt(size_t idx) const
{
  return _code.at(idx);
}
const uint8_t* Chunk::codeBegin() const
{
  return _code.data();
}
