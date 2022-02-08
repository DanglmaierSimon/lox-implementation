#include <vector>

#include "lox/chunk.h"

#include "lox/value.h"

void writeChunk(Chunk* chunk, uint8_t byte, int line)
{
  assert(chunk != nullptr);

  chunk->code.push_back(byte);
  chunk->lines.push_back(line);
}

int addConstant(Chunk* chunk, Value value)
{
  assert(chunk != nullptr);

  chunk->constants.push_back(value);
  return chunk->constants.size() - 1;
}