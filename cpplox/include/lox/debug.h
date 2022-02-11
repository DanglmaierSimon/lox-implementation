#pragma once

#include "chunk.h"

void disassembleChunk(Chunk* chunk, std::string_view name);
int disassembleInstruction(Chunk* chunk, int offset);