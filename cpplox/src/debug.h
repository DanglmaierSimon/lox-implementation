#pragma once

#include "chunk.h"

void disassembleChunk(Chunk* chunk, std::string_view name);
size_t disassembleInstruction(Chunk* chunk, size_t offset);