#include <cassert>

#include "chunk.h"

#include "memory.h"
#include "vm.h"

void initChunk(Chunk* chunk)
{
  assert(chunk != nullptr);

  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = nullptr;
  chunk->lines = nullptr;
  initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line)
{
  assert(chunk != nullptr);

  if (chunk->capacity < chunk->count + 1) {
    const auto oldcap = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldcap);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldcap, chunk->capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, oldcap, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

void freeChunk(Chunk* chunk)
{
  if (chunk == nullptr) {
    return;
  }

  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value)
{
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}