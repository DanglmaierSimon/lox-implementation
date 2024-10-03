#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "compiler.h"

#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "object.h"

#ifdef DEBUG_PRINT_CODE
#  include "debug.h"
#endif

namespace
{
bool identifierEqual(Token* a, Token* b)
{
  if (a->length != b->length) {
    return false;
  }

  return memcmp(a->start, b->start, a->length) == 0;
}

Token syntheticToken(const char* text)
{
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

}  // namespace

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

typedef void (*ParseFn)(bool canAssign, Compiler*);

struct ParseRule
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

// forward declarations
static ParseRule getRule(TokenType type);
static void parsePrecedence(Precedence precedence, Compiler* compiler);

Chunk* Compiler::currentChunk()
{
  return &_function->chunk;
}

void Compiler::emitByte(uint8_t byte)
{
  writeChunk(currentChunk(), byte, _parser->previous.line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
  emitByte(byte1);
  emitByte(byte2);
}

int Compiler::emitJump(uint8_t instruction)
{
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count - 2;
}

void Compiler::emitReturn()
{
  if (functionType == TYPE_INITIALIZER) {
    emitBytes(OP_GET_LOCAL, 0);
  } else {
    // functions without a return value implicitely return nil, except for
    // initializers
    emitByte(OP_NIL);
  }

  emitByte(OP_RETURN);
}

void Compiler::emitLoop(int loopStart)
{
  emitByte(OP_LOOP);

  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX) {
    _parser->error("Loop body too large.");
  }

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

uint8_t Compiler::makeConstant(Value value)
{
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    _parser->error("Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)constant;
}

void Compiler::emitConstant(Value value)
{
  emitBytes(OP_CONSTANT, makeConstant(value));
}

void Compiler::patchJump(int offset)
{
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentChunk()->count - offset - 2;

  if (jump > UINT16_MAX) {
    _parser->error("Too much code to jump over.");
  }

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

void Compiler::markInitialized()
{
  if (scopeDepth == 0) {
    return;  // functions are marked initialised too and may appear in non-local
             // scopes
  }

  locals[localCount - 1].depth = scopeDepth;
}

ObjFunction* Compiler::endCompiler()
{
  emitReturn();
  ObjFunction* _lfunction = this->_function;

#ifdef DEBUG_PRINT_CODE
  if (!_parser->hadError) {
    disassembleChunk(
        currentChunk(),
        _lfunction->name != nullptr ? _lfunction->name->chars : "<script>");
  }
#endif

  return _lfunction;
}

void Compiler::beginScope()
{
  scopeDepth++;
}

void Compiler::endScope()
{
  scopeDepth--;

  while (localCount > 0 && locals[localCount - 1].depth > scopeDepth) {
    if (locals[localCount - 1].isCaptured) {
      emitByte(OP_CLOSE_UPVALUE);
    } else {
      emitByte(OP_POP);
    }
    localCount--;
  }
}

static void binary(bool, Compiler* compiler)
{
  TokenType opType = compiler->_parser->previous.type;
  ParseRule rule = getRule(opType);
  parsePrecedence((Precedence)(rule.precedence + 1), compiler);

  switch (opType) {
    case TokenType::BANG_EQUAL:
      compiler->emitBytes(OP_EQUAL, OP_NOT);
      break;
    case TokenType::EQUAL_EQUAL:
      compiler->emitByte(OP_EQUAL);
      break;
    case TokenType::GREATER:
      compiler->emitByte(OP_GREATER);
      break;
    case TokenType::GREATER_EQUAL:
      compiler->emitBytes(OP_LESS, OP_NOT);
      break;
    case TokenType::LESS:
      compiler->emitByte(OP_LESS);
      break;
    case TokenType::LESS_EQUAL:
      compiler->emitBytes(OP_GREATER, OP_NOT);
      break;
    case TokenType::PLUS:
      compiler->emitByte(OP_ADD);
      break;
    case TokenType::MINUS:
      compiler->emitByte(OP_SUBTRACT);
      break;
    case TokenType::STAR:
      compiler->emitByte(OP_MULTIPLY);
      break;
    case TokenType::SLASH:
      compiler->emitByte(OP_DIVIDE);
      break;
    default:
      return;  // Unreachable
  }
}

uint8_t Compiler::argumentList()
{
  uint8_t argCount = 0;

  if (!_parser->check(TokenType::RIGHT_PAREN)) {
    do {
      expression();

      if (argCount == 255) {
        _parser->error("Can't have more than 255 arguments.");
      }

      argCount++;
    } while (_parser->match(TokenType::COMMA));
  }

  _parser->consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}

static void call(bool, Compiler* compiler)
{
  uint8_t argCount = compiler->argumentList();
  compiler->emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign, Compiler* compiler)
{
  compiler->consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
  uint8_t name = compiler->identifierConstant(&compiler->_parser->previous);

  if (canAssign && compiler->match(TokenType::EQUAL)) {
    compiler->expression();
    compiler->emitBytes(OP_SET_PROPERTY, name);
  } else if (compiler->match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = compiler->argumentList();
    compiler->emitBytes(OP_INVOKE, name);
    compiler->emitByte(argCount);
  } else {
    compiler->emitBytes(OP_GET_PROPERTY, name);
  }
}

static void literal(bool, Compiler* compiler)
{
  switch (compiler->_parser->previous.type) {
    case TokenType::FALSE:
      compiler->emitByte(OP_FALSE);
      break;
    case TokenType::NIL:
      compiler->emitByte(OP_NIL);
      break;
    case TokenType::TRUE:
      compiler->emitByte(OP_TRUE);
      break;
    default:
      return;  // Unreachable
  }
}

void Compiler::expression()
{
  parsePrecedence(PREC_ASSIGNMENT, this);
}

void Compiler::block()
{
  while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE)) {
    declaration();
  }

  consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::function(FunctionType type)
{
  Compiler compiler(_parser, this, type);
  beginScope();

  consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      compiler._function->arity++;
      if (compiler._function->arity > 255) {
        _parser->errorAtCurrent("Can't have more than 255 parameters.");
      }

      uint8_t constant = compiler.parseVariable("Expect parameter name.");
      compiler.defineVariable(constant);
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

void Compiler::method()
{
  consume(TokenType::IDENTIFIER, "Expect method name.");
  uint8_t constant = identifierConstant(&_parser->previous);

  FunctionType type = TYPE_METHOD;

  if (_parser->previous.length == 4
      && memcmp(_parser->previous.start, "init", 4) == 0)
  {
    type = TYPE_INITIALIZER;
  }

  function(type);

  emitBytes(OP_METHOD, constant);
}

void Compiler::funDeclaration()
{
  uint8_t global = parseVariable("Expect function name.");
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}

void Compiler::varDeclaration()
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

void Compiler::expressionStatement()
{
  expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Compiler::forStatement()
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

void Compiler::ifStatement()
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

void Compiler::whileStatement()
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

void Compiler::printStatement()
{
  expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

void Compiler::returnStatement()
{
  if (functionType == TYPE_SCRIPT) {
    _parser->error("Can't return from top-level code.");
  }

  if (match(TokenType::SEMICOLON)) {
    emitReturn();
  } else {
    if (functionType == TYPE_INITIALIZER) {
      _parser->error("Can't return a value from an initializer.");
    }

    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    emitByte(OP_RETURN);
  }
}

void Compiler::synchronize()
{
  _parser->panicMode = false;

  while (_parser->current.type != TokenType::END_OF_FILE) {
    if (_parser->previous.type == TokenType::SEMICOLON) {
      return;
    }

    switch (_parser->current.type) {
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

void and_(bool, Compiler* compiler)
{
  int endJump = compiler->emitJump(OP_JUMP_IF_FALSE);

  compiler->emitByte(OP_POP);
  parsePrecedence(PREC_AND, compiler);
  compiler->patchJump(endJump);
}

void or_(bool, Compiler* compiler)
{
  int elseJump = compiler->emitJump(OP_JUMP_IF_FALSE);
  int endJump = compiler->emitJump(OP_JUMP);

  compiler->patchJump(elseJump);
  compiler->emitByte(OP_POP);

  parsePrecedence(PREC_OR, compiler);
  compiler->patchJump(endJump);
}

void Compiler::namedVariable(Token name, bool canAssign)
{
  uint8_t getOp, setOp;
  int arg = resolveLocal(&name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((arg = resolveUpvalue(&name)) != -1) {
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

void variable(bool canAssign, Compiler* compiler)
{
  compiler->namedVariable(compiler->_parser->previous, canAssign);
}

void super_(bool, Compiler* compiler)
{
  if (compiler->currentClass == nullptr) {
    compiler->error("Can't use 'super' outside of a class.");
  } else if (!compiler->currentClass->hasSuperclass) {
    compiler->error("Can't use 'super' in a class with no superclass.");
  }

  compiler->consume(TokenType::DOT, "Expect '.' after 'super'.");
  compiler->consume(TokenType::IDENTIFIER, "Expect superclass method name.");
  uint8_t name = compiler->identifierConstant(&compiler->_parser->previous);

  compiler->namedVariable(syntheticToken("this"), false);
  if (compiler->match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = compiler->argumentList();
    compiler->namedVariable(syntheticToken("super"), false);
    compiler->emitBytes(OP_SUPER_INVOKE, name);
    compiler->emitByte(argCount);
  } else {
    compiler->namedVariable(syntheticToken("super"), false);
    compiler->emitBytes(OP_GET_SUPER, name);
  }
}

void Compiler::addLocal(Token name)
{
  if (localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }

  Local* local = &locals[localCount++];
  local->name = name;
  local->depth = -1;
  local->isCaptured = false;
}

void Compiler::classDeclaration()
{
  consume(TokenType::IDENTIFIER, "Expect class name.");
  Token className = _parser->previous;
  uint8_t nameconstant = identifierConstant(&_parser->previous);
  declareVariable();

  emitBytes(OP_CLASS, nameconstant);
  defineVariable(nameconstant);

  ClassCompiler classCompiler;
  classCompiler.enclosing = currentClass;
  currentClass = &classCompiler;

  if (match(TokenType::LESS)) {
    consume(TokenType::IDENTIFIER, "Expect superclass name.");
    variable(false, this);

    if (identifierEqual(&className, &_parser->previous)) {
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

void Compiler::declaration()
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

  if (_parser->panicMode) {
    synchronize();
  }
}

void Compiler::statement()
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

void grouping(bool, Compiler* compiler)
{
  compiler->expression();
  compiler->consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool, Compiler* compiler)
{
  double value = strtod(compiler->_parser->previous.start, NULL);

  compiler->emitConstant(NUMBER_VAL(value));
}

static void string(bool, Compiler* compiler)
{
  compiler->emitConstant(
      OBJ_VAL(copyString(compiler->_parser->previous.start + 1,
                         compiler->_parser->previous.length - 2)));
}

static void this_(bool, Compiler* compiler)
{
  if (compiler->currentClass == nullptr) {
    compiler->error("Can't use 'this' outside of a class.");
    return;
  }

  variable(false, compiler);
}

static void unary(bool, Compiler* compiler)
{
  TokenType operatorType = compiler->_parser->previous.type;

  // compile operand
  parsePrecedence(PREC_UNARY, compiler);

  switch (operatorType) {
    case TokenType::BANG:
      compiler->emitByte(OP_NOT);
      break;
    case TokenType::MINUS:
      compiler->emitByte(OP_NEGATE);
      break;
    default:
      return;  // Unreachable
  }
}

static void parsePrecedence(Precedence precedence, Compiler* compiler)
{
  compiler->advance();
  ParseFn prefixRule = getRule(compiler->_parser->previous.type).prefix;
  if (prefixRule == NULL) {
    compiler->error("Expect expression.");
    return;
  }

  const bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign, compiler);

  while (precedence <= getRule(compiler->_parser->current.type).precedence) {
    compiler->advance();
    ParseFn infixRule = getRule(compiler->_parser->previous.type).infix;
    infixRule(canAssign, compiler);
  }

  // invalid target for an assignment leads to the = not being consumed
  // Example: a * b = c + d;
  // this ensures an error is emitted
  if (canAssign && compiler->match(TokenType::EQUAL)) {
    compiler->error("Invalid assignment target.");
  }
}

uint8_t Compiler::identifierConstant(Token* name)
{
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

int Compiler::resolveLocal(Token* name)
{
  for (int i = localCount - 1; i >= 0; i--) {
    Local* local = &locals[i];
    if (identifierEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

int Compiler::addUpvalue(uint8_t index, bool isLocal)
{
  int upvalueCount = _function->upvalueCount;

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue* upvalue = &upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if (upvalueCount == UINT8_COUNT) {
    error("Too many closure variables in function.");
    return 0;
  }

  upvalues[upvalueCount].isLocal = isLocal;
  upvalues[upvalueCount].index = index;
  return _function->upvalueCount++;
}

int Compiler::resolveUpvalue(Token* name)
{
  if (enclosing == nullptr) {
    return -1;
  }

  int local = enclosing->resolveLocal(name);

  if (local != -1) {
    enclosing->locals[local].isCaptured = true;
    return addUpvalue((uint8_t)local, true);
  }

  int upvalue = enclosing->resolveUpvalue(name);
  if (upvalue != -1) {
    return addUpvalue((uint8_t)upvalue, false);
  }

  return -1;
}

void Compiler::declareVariable()
{
  if (scopeDepth == 0) {
    return;
  }

  Token* name = &_parser->previous;

  for (int i = localCount - 1; i >= 0; i--) {
    Local* local = &locals[i];
    if (local->depth != -1 && local->depth < scopeDepth) {
      break;
    }

    if (identifierEqual(name, &local->name)) {
      error("Already a variable with this name in this scope.");
    }
  }

  addLocal(*name);
}

uint8_t Compiler::parseVariable(const char* errorMessage)
{
  consume(TokenType::IDENTIFIER, errorMessage);

  declareVariable();
  if (scopeDepth > 0) {
    return 0;
  }

  return identifierConstant(&_parser->previous);
}

void Compiler::defineVariable(uint8_t global)
{
  if (scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule getRule(TokenType type)
{
  switch (type) {
      // Parentheses ()
    case TokenType::LEFT_PAREN:
      return {grouping, call, PREC_CALL};
    case TokenType::RIGHT_PAREN:
      return {NULL, NULL, PREC_NONE};

      // Braces {}
    case TokenType::LEFT_BRACE:
      return {NULL, NULL, PREC_NONE};
    case TokenType::RIGHT_BRACE:
      return {NULL, NULL, PREC_NONE};

      // Punctuation , .
    case TokenType::COMMA:
      return {NULL, NULL, PREC_NONE};
    case TokenType::DOT:
      return {NULL, dot, PREC_CALL};

    // Mathematical symbols + - / *
    case TokenType::MINUS:
      return {unary, binary, PREC_TERM};
    case TokenType::PLUS:
      return {NULL, binary, PREC_TERM};
    case TokenType::SLASH:
      return {NULL, binary, PREC_FACTOR};
    case TokenType::STAR:
      return {NULL, binary, PREC_FACTOR};

    // Semicolon ;
    case TokenType::SEMICOLON:
      return {NULL, NULL, PREC_NONE};
    // Assignment =
    case TokenType::EQUAL:
      return {NULL, NULL, PREC_NONE};

    // Not operator !
    case TokenType::BANG:
      return {unary, NULL, PREC_NONE};

    // Comparison operators ! != = == > >= < <=
    case TokenType::BANG_EQUAL:
      return {NULL, binary, PREC_EQUALITY};
    case TokenType::EQUAL_EQUAL:
      return {NULL, binary, PREC_EQUALITY};
    case TokenType::GREATER:
      return {NULL, binary, PREC_COMPARISON};
    case TokenType::GREATER_EQUAL:
      return {NULL, binary, PREC_COMPARISON};
    case TokenType::LESS:
      return {NULL, binary, PREC_COMPARISON};
    case TokenType::LESS_EQUAL:
      return {NULL, binary, PREC_COMPARISON};

    // Boolean operators and or
    case TokenType::AND:
      return {NULL, and_, PREC_AND};
    case TokenType::OR:
      return {NULL, or_, PREC_OR};

      // Keywords
    case TokenType::CLASS:
      return {NULL, NULL, PREC_NONE};
    case TokenType::ELSE:
      return {NULL, NULL, PREC_NONE};
    case TokenType::FOR:
      return {NULL, NULL, PREC_NONE};
    case TokenType::FUN:
      return {NULL, NULL, PREC_NONE};
    case TokenType::IF:
      return {NULL, NULL, PREC_NONE};
    case TokenType::RETURN:
      return {NULL, NULL, PREC_NONE};
    case TokenType::SUPER:
      return {super_, NULL, PREC_NONE};
    case TokenType::THIS:
      return {this_, NULL, PREC_NONE};
    case TokenType::VAR:
      return {NULL, NULL, PREC_NONE};
    case TokenType::WHILE:
      return {NULL, NULL, PREC_NONE};

    // Built-in functions
    case TokenType::PRINT:
      return {NULL, NULL, PREC_NONE};

    // Literals
    case TokenType::FALSE:
      return {literal, NULL, PREC_NONE};
    case TokenType::NIL:
      return {literal, NULL, PREC_NONE};
    case TokenType::TRUE:
      return {literal, NULL, PREC_NONE};

    // Values
    case TokenType::STRING:
      return {string, NULL, PREC_NONE};
    case TokenType::NUMBER:
      return {number, NULL, PREC_NONE};

    case TokenType::IDENTIFIER:
      return {variable, NULL, PREC_NONE};

    case TokenType::ERROR_TOKEN:
      return {NULL, NULL, PREC_NONE};
    case TokenType::END_OF_FILE:
      return {NULL, NULL, PREC_NONE};
  };
}

ObjFunction* Compiler::compile()
{
  _parser->hadError = false;
  _parser->panicMode = false;

  advance();

  while (!match(TokenType::END_OF_FILE)) {
    declaration();
  }

  auto function = endCompiler();
  return _parser->hadError ? nullptr : function;
}

void Compiler::markRoots()
{
  Compiler* compiler = this;
  while (compiler != nullptr) {
    markObject((Obj*)compiler->_function);
    compiler = compiler->enclosing;
  }
}

Compiler::Compiler(std::shared_ptr<Parser> parser,
                   Compiler* current,
                   FunctionType type)
    : _parser(parser)
{
  enclosing = current;

  _function = nullptr;
  functionType = type;
  localCount = 0;
  scopeDepth = 0;
  _function = newFunction();

  if (type != TYPE_SCRIPT) {
    _function->name =
        copyString(_parser->previous.start, _parser->previous.length);
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

bool Compiler::check(TokenType type)
{
  return _parser->check(type);
}

bool Compiler::match(TokenType type)
{
  return _parser->match(type);
}

void Compiler::advance()
{
  _parser->advance();
}

void Compiler::consume(TokenType type, const char* message)
{
  _parser->consume(type, message);
}

void Compiler::errorAt(Token* token, const char* message)
{
  _parser->errorAt(token, message);
}

void Compiler::errorAtCurrent(const char* message)
{
  _parser->errorAtCurrent(message);
}

void Compiler::error(const char* message)
{
  _parser->error(message);
}