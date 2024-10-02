#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "compiler.h"

#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#  include "debug.h"
#endif

struct Parser
{
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
};

enum Precedence
{
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,  // or
  PREC_AND,  // and
  PREC_EQUALITY,  // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,  // + -
  PREC_FACTOR,  // * /
  PREC_UNARY,  // ! -
  PREC_CALL,  // . ()
  PREC_PRIMARY
};

typedef void (*ParseFn)(bool canAssign);

struct ParseRule
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
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

enum FunctionType
{
  TYPE_FUNCTION,
  TYPE_METHOD,
  TYPE_SCRIPT,
  TYPE_INITIALIZER,
};

struct Compiler
{
  Compiler(std::shared_ptr<Scanner> scanner)
      : _scanner(std::move(scanner))
  {
  }

  std::shared_ptr<Scanner> _scanner;

  Compiler* enclosing;
  ObjFunction* function;
  FunctionType type;

  Local locals[UINT8_COUNT];
  int localCount;
  Upvalue upvalues[UINT8_COUNT];

  int scopeDepth;
};

struct ClassCompiler
{
  bool hasSuperclass = false;
  ClassCompiler* enclosing = nullptr;
};

Chunk* compilingChunk;
Parser parser;
Compiler* current = nullptr;
ClassCompiler* currentClass = nullptr;

// forward declarations
static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static void defineVariable(uint8_t global);
static uint8_t identifierConstant(Token* name);
static uint8_t parseVariable(const char* errorMessage);
static int resolveLocal(Compiler* compiler, Token* name);
static int resolveUpvalue(Compiler* compiler, Token* name);
static void declareVariable();
static void namedVariable(Token name, bool canAssign);

static Chunk* currentChunk()
{
  return &current->function->chunk;
}

static void errorAt(Token* token, const char* message)
{
  assert(token != nullptr);

  if (parser.panicMode) {
    // suppress subsequent errors in panic mode to prevent error cascades
    return;
  }

  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::END_OF_FILE) {
    fprintf(stderr, " at end");
  } else if (token->type == TokenType::ERROR_TOKEN) {
    // do nothing;
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

static void errorAtCurrent(const char* message)
{
  errorAt(&parser.current, message);
}

static void error(const char* message)
{
  errorAt(&parser.previous, message);
}

static void advance()
{
  parser.previous = parser.current;

  while (true) {
    parser.current = current->_scanner->scanToken();
    if (parser.current.type != TokenType::ERROR_TOKEN) {
      break;
    }

    errorAtCurrent(parser.current.start);
  }
}

static void consume(TokenType type, const char* message)
{
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

static bool check(TokenType type)
{
  return parser.current.type == type;
}

static bool match(TokenType type)
{
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}

static bool identifierEqual(Token* a, Token* b)
{
  if (a->length != b->length) {
    return false;
  }

  return memcmp(a->start, b->start, a->length) == 0;
}

static void emitByte(uint8_t byte)
{
  writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
  emitByte(byte1);
  emitByte(byte2);
}

static int emitJump(uint8_t instruction)
{
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count - 2;
}

static void emitReturn()
{
  if (current->type == TYPE_INITIALIZER) {
    emitBytes(OP_GET_LOCAL, 0);
  } else {
    // functions without a return value implicitely return nil, except for
    // initializers
    emitByte(OP_NIL);
  }

  emitByte(OP_RETURN);
}

static void emitLoop(int loopStart)
{
  emitByte(OP_LOOP);

  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX) {
    error("Loop body too large.");
  }

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

static uint8_t makeConstant(Value value)
{
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)constant;
}

static void emitConstant(Value value)
{
  emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset)
{
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentChunk()->count - offset - 2;

  if (jump > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

static void markInitialized()
{
  if (current->scopeDepth == 0) {
    return;  // functions are marked initialised too and may appear in non-local
             // scopes
  }

  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void initCompiler(Compiler* compiler, FunctionType type)
{
  compiler->enclosing = current;

  compiler->function = nullptr;
  compiler->type = type;
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  compiler->function = newFunction();
  current = compiler;

  if (type != TYPE_SCRIPT) {
    current->function->name =
        copyString(parser.previous.start, parser.previous.length);
  }

  Local* local = &current->locals[current->localCount++];
  local->depth = 0;
  local->isCaptured = false;

  if (type != TYPE_FUNCTION) {
    local->name.start = "this";
    local->name.length = 4;
  } else {
    local->name.start = "";
    local->name.length = 0;
  }
}

static ObjFunction* endCompiler()
{
  emitReturn();
  ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(
        currentChunk(),
        function->name != nullptr ? function->name->chars : "<script>");
  }
#endif

  current = current->enclosing;
  return function;
}

static void beginScope()
{
  current->scopeDepth++;
}

static void endScope()
{
  current->scopeDepth--;

  while (current->localCount > 0
         && current->locals[current->localCount - 1].depth
                > current->scopeDepth)
  {
    if (current->locals[current->localCount - 1].isCaptured) {
      emitByte(OP_CLOSE_UPVALUE);
    } else {
      emitByte(OP_POP);
    }
    current->localCount--;
  }
}

static void binary(bool)
{
  TokenType opType = parser.previous.type;
  ParseRule* rule = getRule(opType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (opType) {
    case TokenType::BANG_EQUAL:
      emitBytes(OP_EQUAL, OP_NOT);
      break;
    case TokenType::EQUAL_EQUAL:
      emitByte(OP_EQUAL);
      break;
    case TokenType::GREATER:
      emitByte(OP_GREATER);
      break;
    case TokenType::GREATER_EQUAL:
      emitBytes(OP_LESS, OP_NOT);
      break;
    case TokenType::LESS:
      emitByte(OP_LESS);
      break;
    case TokenType::LESS_EQUAL:
      emitBytes(OP_GREATER, OP_NOT);
      break;
    case TokenType::PLUS:
      emitByte(OP_ADD);
      break;
    case TokenType::MINUS:
      emitByte(OP_SUBTRACT);
      break;
    case TokenType::STAR:
      emitByte(OP_MULTIPLY);
      break;
    case TokenType::SLASH:
      emitByte(OP_DIVIDE);
      break;
    default:
      return;  // Unreachable
  }
}

static uint8_t argumentList()
{
  uint8_t argCount = 0;

  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      expression();

      if (argCount == 255) {
        error("Can't have more than 255 arguments.");
      }

      argCount++;
    } while (match(TokenType::COMMA));
  }

  consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}

static void call(bool)
{
  uint8_t argCount = argumentList();
  emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign)
{
  consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
  uint8_t name = identifierConstant(&parser.previous);

  if (canAssign && match(TokenType::EQUAL)) {
    expression();
    emitBytes(OP_SET_PROPERTY, name);
  } else if (match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    emitBytes(OP_INVOKE, name);
    emitByte(argCount);
  } else {
    emitBytes(OP_GET_PROPERTY, name);
  }
}

static void literal(bool)
{
  switch (parser.previous.type) {
    case TokenType::FALSE:
      emitByte(OP_FALSE);
      break;
    case TokenType::NIL:
      emitByte(OP_NIL);
      break;
    case TokenType::TRUE:
      emitByte(OP_TRUE);
      break;
    default:
      return;  // Unreachable
  }
}

static void expression()
{
  parsePrecedence(PREC_ASSIGNMENT);
}

static void block()
{
  while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE)) {
    declaration();
  }

  consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type)
{
  Compiler compiler(current->_scanner);
  initCompiler(&compiler, type);
  beginScope();

  consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      current->function->arity++;
      if (current->function->arity > 255) {
        errorAtCurrent("Can't have more than 255 parameters.");
      }

      uint8_t constant = parseVariable("Expect parameter name.");
      defineVariable(constant);
    } while (match(TokenType::COMMA));
  }

  consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
  consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
  block();

  auto* function = endCompiler();
  emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

  for (int i = 0; i < function->upvalueCount; i++) {
    emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
    emitByte(compiler.upvalues[i].index);
  }
}

static void method()
{
  consume(TokenType::IDENTIFIER, "Expect method name.");
  uint8_t constant = identifierConstant(&parser.previous);

  FunctionType type = TYPE_METHOD;

  if (parser.previous.length == 4
      && memcmp(parser.previous.start, "init", 4) == 0)
  {
    type = TYPE_INITIALIZER;
  }

  function(type);

  emitBytes(OP_METHOD, constant);
}

static void funDeclaration()
{
  uint8_t global = parseVariable("Expect function name.");
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}

static void varDeclaration()
{
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TokenType::EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }

  consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
  defineVariable(global);
}

static void expressionStatement()
{
  expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void forStatement()
{
  beginScope();

  consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(TokenType::SEMICOLON)) {
    // no initializer -> do nothing
  } else if (match(TokenType::VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }

  int loopStart = currentChunk()->count;
  int exitJump = -1;

  if (!match(TokenType::SEMICOLON)) {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    // Jump out of the loop if the condition is false
    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }

  // Explanation of the code for the increment part of the for loop, taken
  // directly from the book.

  // Again, it’s optional. Since this is the last
  // clause, when omitted, the next token will be the closing parenthesis.
  // When an increment is present, we need to compile it now, but it shouldn’t
  // execute yet. So, first, we emit an unconditional jump that hops over the
  // increment clause’s code to the body of the loop. Next, we compile the
  // increment expression itself. This is usually an assignment. Whatever it
  // is, we only execute it for its side effect, so we also emit a pop to
  // discard its value. The last part is a little tricky. First, we emit a
  // loop instruction. This is the main loop that takes us back to the top of
  // the for loop—right before the condition expression if there is one. That
  // loop happens right after the increment, since the increment executes at
  // the end of each loop iteration. Then we change loopStart to point to the
  // offset where the increment expression begins. Later, when we emit the
  // loop instruction after the body statement, this will cause it to jump up
  // to the increment expression instead of the top of the loop like it does
  // when there is no increment. This is how we weave the increment in to run
  // after the body.

  /* Wonderful diagram that was a pain in the ass to make
       ┌──────────────────┐
       │Initializer clause│
       └──────────────────┘

       ┌────────────────────┐◄─┐
       │Condition expression│  │
       └────────────────────┘  │
                               │
    ┌───OP_JUMP_IF_FALSE       │
    │                          │
    │   OP_POP                 │
    │                          │
  ┌─┼───OP_JUMP                │
  │ │                          │
  │ │  ┌────────────────────┐◄─┼─┐
  │ │  │Increment expression│  │ │
  │ │  └────────────────────┘  │ │
  │ │                          │ │
  │ │   OP_POP                 │ │
  │ │                          │ │
  │ │   OP_LOOP ───────────────┘ │
  │ │                            │
  └─┼─►┌──────────────┐          │
    │  │Body statement│          │
    │  └──────────────┘          │
    │                            │
    │   OP_LOOP ─────────────────┘
    └──►
        OP_POP
*/

  if (!match(TokenType::RIGHT_PAREN)) {
    int bodyJump =
        emitJump(OP_JUMP);  // unconditionally jump over increment clause
    int incrementStart = currentChunk()->count;
    expression();  // compile increment clause
    emitByte(OP_POP);  // expression only executed for sideeffect, pop value
                       // off stack

    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(loopStart);
    loopStart = incrementStart;
    patchJump(bodyJump);
  }

  statement();
  emitLoop(loopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP);
  }

  endScope();
}

static void ifStatement()
{
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

  const int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  const int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);

  emitByte(OP_POP);

  if (match(TokenType::ELSE)) {
    statement();
  }

  patchJump(elseJump);
}

static void whileStatement()
{
  int loopStart = currentChunk()->count;

  consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  emitLoop(loopStart);

  patchJump(exitJump);
  emitByte(OP_POP);
}

static void printStatement()
{
  expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

static void returnStatement()
{
  if (current->type == TYPE_SCRIPT) {
    error("Can't return from top-level code.");
  }

  if (match(TokenType::SEMICOLON)) {
    emitReturn();
  } else {
    if (current->type == TYPE_INITIALIZER) {
      error("Can't return a value from an initializer.");
    }

    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    emitByte(OP_RETURN);
  }
}

static void synchronize()
{
  parser.panicMode = false;

  while (parser.current.type != TokenType::END_OF_FILE) {
    if (parser.previous.type == TokenType::SEMICOLON) {
      return;
    }

    switch (parser.current.type) {
      case TokenType::CLASS:
      case TokenType::FUN:
      case TokenType::VAR:
      case TokenType::FOR:
      case TokenType::IF:
      case TokenType::WHILE:
      case TokenType::PRINT:
      case TokenType::RETURN:
        return;
      default:
        break;  // do nothing
    }

    advance();
  }
}

static void and_(bool)
{
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(PREC_AND);
  patchJump(endJump);
}

static void or_(bool)
{
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);
}

static void namedVariable(Token name, bool canAssign)
{
  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((arg = resolveUpvalue(current, &name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TokenType::EQUAL)) {
    expression();
    emitBytes(setOp, static_cast<uint8_t>(arg));
  } else {
    emitBytes(getOp, static_cast<uint8_t>(arg));
  }
}

static void variable(bool canAssign)
{
  namedVariable(parser.previous, canAssign);
}

static Token syntheticToken(const char* text)
{
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

static void super_(bool)
{
  if (currentClass == nullptr) {
    error("Can't use 'super' outside of a class.");
  } else if (!currentClass->hasSuperclass) {
    error("Can't use 'super' in a class with no superclass.");
  }

  consume(TokenType::DOT, "Expect '.' after 'super'.");
  consume(TokenType::IDENTIFIER, "Expect superclass method name.");
  uint8_t name = identifierConstant(&parser.previous);

  namedVariable(syntheticToken("this"), false);
  if (match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_SUPER_INVOKE, name);
    emitByte(argCount);
  } else {
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_GET_SUPER, name);
  }
}

static void addLocal(Token name)
{
  if (current->localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }

  Local* local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = -1;
  local->isCaptured = false;
}

static void classDeclaration()
{
  consume(TokenType::IDENTIFIER, "Expect class name.");
  Token className = parser.previous;
  uint8_t nameconstant = identifierConstant(&parser.previous);
  declareVariable();

  emitBytes(OP_CLASS, nameconstant);
  defineVariable(nameconstant);

  ClassCompiler classCompiler;
  classCompiler.enclosing = currentClass;
  currentClass = &classCompiler;

  if (match(TokenType::LESS)) {
    consume(TokenType::IDENTIFIER, "Expect superclass name.");
    variable(false);

    if (identifierEqual(&className, &parser.previous)) {
      error("A class can't inherit from itself.");
    }

    beginScope();
    addLocal(syntheticToken("super"));
    defineVariable(0);

    namedVariable(className, false);
    emitByte(OP_INHERIT);
    classCompiler.hasSuperclass = true;
  }

  namedVariable(className, false);

  consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

  while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE)) {
    method();
  }

  consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
  emitByte(OP_POP);

  if (classCompiler.hasSuperclass) {
    endScope();
  }

  currentClass = currentClass->enclosing;
}

static void declaration()
{
  if (match(TokenType::CLASS)) {
    classDeclaration();
  } else if (match(TokenType::FUN)) {
    funDeclaration();
  } else if (match(TokenType::VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode) {
    synchronize();
  }
}

static void statement()
{
  if (match(TokenType::PRINT)) {
    printStatement();
  } else if (match(TokenType::FOR)) {
    forStatement();
  } else if (match(TokenType::IF)) {
    ifStatement();
  } else if (match(TokenType::RETURN)) {
    returnStatement();
  } else if (match(TokenType::WHILE)) {
    whileStatement();
  } else if (match(TokenType::LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

static void grouping(bool)
{
  expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool)
{
  double value = strtod(parser.previous.start, NULL);

  emitConstant(NUMBER_VAL(value));
}

static void string(bool)
{
  emitConstant(OBJ_VAL(
      copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void this_(bool)
{
  if (currentClass == nullptr) {
    error("Can't use 'this' outside of a class.");
    return;
  }

  variable(false);
}

static void unary(bool)
{
  TokenType operatorType = parser.previous.type;

  // compile operand
  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
    case TokenType::BANG:
      emitByte(OP_NOT);
      break;
    case TokenType::MINUS:
      emitByte(OP_NEGATE);
      break;
    default:
      return;  // Unreachable
  }
}

static std::unordered_map<TokenType, ParseRule> rules {
    // Parentheses ()
    {TokenType::LEFT_PAREN, {grouping, call, PREC_CALL}},
    {TokenType::RIGHT_PAREN, {NULL, NULL, PREC_NONE}},

    // Braces {}
    {TokenType::LEFT_BRACE, {NULL, NULL, PREC_NONE}},
    {TokenType::RIGHT_BRACE, {NULL, NULL, PREC_NONE}},

    // Punctuation , .
    {TokenType::COMMA, {NULL, NULL, PREC_NONE}},
    {TokenType::DOT, {NULL, dot, PREC_CALL}},

    // Mathematical symbols + - / *
    {TokenType::MINUS, {unary, binary, PREC_TERM}},
    {TokenType::PLUS, {NULL, binary, PREC_TERM}},
    {TokenType::SLASH, {NULL, binary, PREC_FACTOR}},
    {TokenType::STAR, {NULL, binary, PREC_FACTOR}},

    // Semicolon ;
    {TokenType::SEMICOLON, {NULL, NULL, PREC_NONE}},

    // Assignment =
    {TokenType::EQUAL, {NULL, NULL, PREC_NONE}},

    // Not operator !
    {TokenType::BANG, {unary, NULL, PREC_NONE}},

    // Comparison operators ! != = == > >= < <=
    {TokenType::BANG_EQUAL, {NULL, binary, PREC_EQUALITY}},
    {TokenType::EQUAL_EQUAL, {NULL, binary, PREC_EQUALITY}},
    {TokenType::GREATER, {NULL, binary, PREC_COMPARISON}},
    {TokenType::GREATER_EQUAL, {NULL, binary, PREC_COMPARISON}},
    {TokenType::LESS, {NULL, binary, PREC_COMPARISON}},
    {TokenType::LESS_EQUAL, {NULL, binary, PREC_COMPARISON}},

    // Boolean operators and or
    {TokenType::AND, {NULL, and_, PREC_AND}},
    {TokenType::OR, {NULL, or_, PREC_OR}},

    // Keywords
    {TokenType::CLASS, {NULL, NULL, PREC_NONE}},
    {TokenType::ELSE, {NULL, NULL, PREC_NONE}},
    {TokenType::FOR, {NULL, NULL, PREC_NONE}},
    {TokenType::FUN, {NULL, NULL, PREC_NONE}},
    {TokenType::IF, {NULL, NULL, PREC_NONE}},
    {TokenType::RETURN, {NULL, NULL, PREC_NONE}},
    {TokenType::SUPER, {super_, NULL, PREC_NONE}},
    {TokenType::THIS, {this_, NULL, PREC_NONE}},
    {TokenType::VAR, {NULL, NULL, PREC_NONE}},
    {TokenType::WHILE, {NULL, NULL, PREC_NONE}},

    // Built-in functions
    {TokenType::PRINT, {NULL, NULL, PREC_NONE}},

    // Literals
    {TokenType::FALSE, {literal, NULL, PREC_NONE}},
    {TokenType::NIL, {literal, NULL, PREC_NONE}},
    {TokenType::TRUE, {literal, NULL, PREC_NONE}},

    // Values
    {TokenType::STRING, {string, NULL, PREC_NONE}},
    {TokenType::NUMBER, {number, NULL, PREC_NONE}},

    {TokenType::IDENTIFIER, {variable, NULL, PREC_NONE}},

    {TokenType::ERROR_TOKEN, {NULL, NULL, PREC_NONE}},
    {TokenType::END_OF_FILE, {NULL, NULL, PREC_NONE}},
};

static void parsePrecedence(Precedence precedence)
{
  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
    return;
  }

  const bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule(canAssign);
  }

  // invalid target for an assignment leads to the = not being consumed
  // Example: a * b = c + d;
  // this ensures an error is emitted
  if (canAssign && match(TokenType::EQUAL)) {
    error("Invalid assignment target.");
  }
}

static uint8_t identifierConstant(Token* name)
{
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

// TODO: change to optional
static int resolveLocal(Compiler* compiler, Token* name)
{
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifierEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

static int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal)
{
  int upvalueCount = compiler->function->upvalueCount;

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if (upvalueCount == UINT8_COUNT) {
    error("Too many closure variables in function.");
    return 0;
  }

  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;
  return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name)
{
  if (compiler->enclosing == nullptr) {
    return -1;
  }

  int local = resolveLocal(compiler->enclosing, name);

  if (local != -1) {
    compiler->enclosing->locals[local].isCaptured = true;
    return addUpvalue(compiler, (uint8_t)local, true);
  }

  int upvalue = resolveUpvalue(compiler->enclosing, name);
  if (upvalue != -1) {
    return addUpvalue(compiler, (uint8_t)upvalue, false);
  }

  return -1;
}

static void declareVariable()
{
  if (current->scopeDepth == 0) {
    return;
  }

  Token* name = &parser.previous;

  for (int i = current->localCount - 1; i >= 0; i--) {
    Local* local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break;
    }

    if (identifierEqual(name, &local->name)) {
      error("Already a variable with this name in this scope.");
    }
  }

  addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage)
{
  consume(TokenType::IDENTIFIER, errorMessage);

  declareVariable();
  if (current->scopeDepth > 0) {
    return 0;
  }

  return identifierConstant(&parser.previous);
}

static void defineVariable(uint8_t global)
{
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule* getRule(TokenType type)
{
  return &rules[type];
}

ObjFunction* compile(const char* source)
{
  auto scanner = std::make_shared<Scanner>(source);

  Compiler compiler(scanner);
  initCompiler(&compiler, TYPE_SCRIPT);

  parser.hadError = false;
  parser.panicMode = false;

  advance();

  while (!match(TokenType::END_OF_FILE)) {
    declaration();
  }

  auto function = endCompiler();
  return parser.hadError ? nullptr : function;
}

void markCompilerRoots()
{
  Compiler* compiler = current;
  while (compiler != nullptr) {
    markObject((Obj*)compiler->function);
    compiler = compiler->enclosing;
  }
}