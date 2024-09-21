#pragma once

import chunk;

void disassembleChunk(Chunk* chunk, std::string_view name);
size_t disassembleInstruction(Chunk* chunk, size_t offset);