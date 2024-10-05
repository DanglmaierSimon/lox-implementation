#pragma once

#include <memory>

#include "object.h"
#include "parser.h"
#include "vm.h"

enum FunctionType
{
  TYPE_FUNCTION,
  TYPE_METHOD,
  TYPE_SCRIPT,
  TYPE_INITIALIZER,
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

struct ClassCompiler
{
  bool hasSuperclass = false;
  ClassCompiler* enclosing = nullptr;
};

class Compiler
{
public:
  Compiler(std::shared_ptr<Parser> parser,
           Compiler* current,
           FunctionType type);

  Compiler(std::shared_ptr<Parser> parser, FunctionType type);

  ObjFunction* compile();
  void markRoots();

  bool check(TokenType type);
  bool match(TokenType type);
  void advance();
  void consume(TokenType type, const char* message);

  void errorAt(Token* token, const char* message);
  void errorAtCurrent(const char* message);
  void error(const char* message);

  void emitByte(uint8_t byte);
  void emitBytes(uint8_t byte1, uint8_t byte2);
  int emitJump(uint8_t instruction);
  void emitReturn();
  void emitLoop(int loopStart);
  void emitConstant(Value value);

  void patchJump(int offset);

  void markInitialized();

  uint8_t makeConstant(Value value);

  Chunk* currentChunk();

  ObjFunction* endCompiler();

  void beginScope();
  void endScope();

  uint8_t argumentList();

  void declaration();
  void block();
  void function(FunctionType type);
  void method();
  void funDeclaration();
  void varDeclaration();
  void expressionStatement();
  void forStatement();
  void ifStatement();
  void whileStatement();
  void printStatement();
  void returnStatement();
  void classDeclaration();
  void statement();
  void expression();

  uint8_t identifierConstant(const Token& name);

  uint8_t parseVariable(const char* errorMessage);

  void synchronize();

  void namedVariable(Token name, bool canAssign);

  void addLocal(Token name);

  void declareVariable();
  void defineVariable(uint8_t global);

  int resolveLocal(Token* name);

  int addUpvalue(uint8_t index, bool isLocal);
  int resolveUpvalue(Token* name);

  std::shared_ptr<Parser> _parser;
  ClassCompiler* currentClass = nullptr;

private:
  Compiler* enclosing = nullptr;
  ObjFunction* _function = nullptr;
  FunctionType functionType;

  Local locals[UINT8_COUNT];
  int localCount;
  Upvalue upvalues[UINT8_COUNT];

  int scopeDepth;
};
