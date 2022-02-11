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

class Compiler;

struct Local
{
  Token name;
  int depth = 0;
  bool isCaptured = false;
};

struct Upvalue
{
  uint8_t index;
  bool isLocal;
};

typedef void (Compiler::*ParseFn)(bool);

struct ParseRule
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

struct ClassCompiler
{
  bool hasSuperclass = false;
  ClassCompiler* enclosing = nullptr;
};

class Compiler
{
public:
  explicit Compiler(Compiler* enclosing,
                    MemoryManager* memory_manager,
                    Parser parser,
                    FunctionType type);

  ~Compiler();

  ObjFunction* compile();

  MemoryManager* memoryManager();
  Compiler* enclosing();
  ObjFunction* function();

private:
  void emitByte(uint8_t byte);
  void emitBytes(uint8_t byte1, uint8_t byte2);
  void emitReturn();
  void emitLoop(int loopStart);
  void emitConstant(Value value);
  int emitJump(uint8_t instruction);
  void patchJump(size_t offset);

  uint8_t identifierConstant(Token name);
  uint8_t makeConstant(Value value);

  void declareVariable();
  void defineVariable(uint8_t global);
  uint8_t parseVariable(const char* errorMessage);
  void namedVariable(Token name, bool canAssign);

  int addUpvalue(uint8_t index, bool isLocal);
  void addLocal(Token name);

  // TODO: Remove compiler parameter
  int resolveUpvalue(Token name);
  int resolveLocal(Token name);

  void beginScope();
  void endScope();

  void markInitialized();

  Chunk* currentChunk();

  ObjFunction* compileFunction();

  ObjFunction* endCompiler();

  ParseRule getRule(TokenType t);
  void parsePrecedence(Precedence precedence);

  void grouping(bool);
  void variable(bool);
  void binary(bool);
  void unary(bool);
  void call(bool);
  void dot(bool canAssign);
  void literal(bool);
  void super_(bool);
  void number(bool);
  void string_(bool);
  void this_(bool);
  void and_(bool);
  void or_(bool);

  void varDeclaration();
  void expressionStatement();
  void forStatement();
  void ifStatement();
  void whileStatement();
  void printStatement();
  void returnStatement();
  void statement();
  void expression();
  void block();
  void function_(FunctionType t);
  void method();
  void classDeclaration();
  uint8_t argumentList();
  void funDeclaration();
  void declaration();

public:
private:
  Compiler* _enclosing = nullptr;
  ObjFunction* _function = nullptr;

  int scopeDepth;
  int localCount;

  Local locals[UINT8_COUNT];
  Upvalue upvalues[UINT8_COUNT];
  FunctionType type;

  MemoryManager* _mm = nullptr;
  ClassCompiler* _currentClass = nullptr;
  Parser parser;
};
