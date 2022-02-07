#pragma once

#include <functional>

#include "lox/objects/objfunction.h"
#include "lox/parser.h"
#include "lox/scanner.h"
#include "vm.h"

enum class FunctionType
{
  FUNCTION,
  METHOD,
  SCRIPT,
  INITIALIZER,
};

enum class Precedence : int
{
  NONE = 0,
  ASSIGNMENT,  // =
  OR,  // or
  AND,  // and
  EQUALITY,  // == !=
  COMPARISON,  // < > <= >=
  TERM,  // + -
  FACTOR,  // * /
  UNARY,  // ! -
  CALL,  // . ()
  PRIMARY
};

struct Local
{
  Token name;
  int depth;
  bool isCaptured;
};

struct Upvalue
{
  uint8_t index;
  bool isLocal;
};

using ParseFn = std::function<void(bool)>;

struct ParseRule
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

class Compiler
{
public:
  explicit Compiler(Compiler* enclosing,
                    MemoryManager* memory_manager,
                    Parser* parser,
                    FunctionType type);

  void emitByte(uint8_t byte);
  void emitBytes(uint8_t byte1, uint8_t byte2);
  int emitJump(uint8_t instruction);
  void emitReturn();
  void emitLoop(int loopStart);
  void emitConstant(Value value);

  uint8_t identifierConstant(Token name);
  uint8_t makeConstant(Value value);

  void declareVariable();
  void defineVariable(uint8_t global);

  void parsePrecedence(Precedence precedence);
  uint8_t parseVariable(const char* errorMessage);

  int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal);
  void addLocal(Token name);

  // TODO: Remove compiler parameter
  int resolveUpvalue(Compiler* compiler, Token name);
  int resolveLocal(Compiler* compiler, Token name);

  void beginScope();
  void endScope();

  void markInitialized();

  Chunk* currentChunk();
  void patchJump(int offset);

  ObjFunction* endCompiler();

private:
public:
  Compiler* enclosing = nullptr;
  ObjFunction* function = nullptr;
  FunctionType type;

  Local locals[UINT8_COUNT];
  int localCount;
  Upvalue upvalues[UINT8_COUNT];

  int scopeDepth;

  MemoryManager* mm = nullptr;
  Parser* parser = nullptr;
};

ObjFunction* compile(MemoryManager* memory_manager, std::string_view source);
void markCompilerRoots();