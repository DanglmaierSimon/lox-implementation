#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "lox/compiler.h"

#include "lox/chunk.h"
#include "lox/common.h"
#include "lox/memory.h"
#include "lox/parser.h"
#include "lox/scanner.h"
#include "lox/value.h"

#ifdef DEBUG_PRINT_CODE
#  include "lox/debug.h"
#endif

namespace
{
bool identifierEqual(Token a, Token b)
{
  return a.string() == b.string();
}

Token syntheticToken(std::string_view text)
{
  return Token {TokenType::ERROR, text, 0};
}

}  // namespace

// forward declaration
static ParseRule getRule(TokenType type);
static void declaration();
static void statement();

struct ClassCompiler
{
  bool hasSuperclass = false;
  ClassCompiler* enclosing = nullptr;
};

Compiler* current = nullptr;
ClassCompiler* currentClass = nullptr;

static void expression()
{
  current->parsePrecedence(Precedence::ASSIGNMENT);
}

static void namedVariable(Token name, bool canAssign)
{
  uint8_t getOp, setOp;
  auto arg = current->resolveLocal(current, name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((arg = current->resolveUpvalue(current, name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  } else {
    arg = current->identifierConstant(name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && current->parser->match(TokenType::EQUAL)) {
    expression();
    current->emitBytes(setOp, static_cast<uint8_t>(arg));
  } else {
    current->emitBytes(getOp, static_cast<uint8_t>(arg));
  }
}

static void variable(bool canAssign)
{
  namedVariable(current->parser->previous(), canAssign);
}

static void block()
{
  while (!current->parser->check(TokenType::RIGHT_BRACE)
         && !current->parser->check(TokenType::END_OF_FILE))
  {
    declaration();
  }

  current->parser->consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}
static void function(FunctionType type)
{
  Compiler compiler;
  initCompiler(&compiler, current->mm, current->parser, type);
  current->beginScope();

  current->parser->consume(TokenType::LEFT_PAREN,
                           "Expect '(' after function name.");

  if (!current->parser->check(TokenType::RIGHT_PAREN)) {
    do {
      current->function->incrementArity();
      if (current->function->arity() > 255) {
        current->parser->errorAtCurrent("Can't have more than 255 parameters.");
      }

      uint8_t constant = current->parseVariable("Expect parameter name.");
      current->defineVariable(constant);
    } while (current->parser->match(TokenType::COMMA));
  }

  current->parser->consume(TokenType::RIGHT_PAREN,
                           "Expect ')' after parameters.");
  current->parser->consume(TokenType::LEFT_BRACE,
                           "Expect '{' before function body.");
  block();

  auto* function = endCompiler();
  current->emitBytes(OP_CLOSURE, current->makeConstant(Value(function)));

  for (int i = 0; i < function->upvalueCount(); i++) {
    current->emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
    current->emitByte(compiler.upvalues[i].index);
  }
}

static void method()
{
  current->parser->consume(TokenType::IDENTIFIER, "Expect method name.");
  uint8_t constant = current->identifierConstant(current->parser->previous());

  FunctionType type = FunctionType::METHOD;

  if (current->parser->previous().string() == "init") {
    type = FunctionType::INITIALIZER;
  }

  function(type);

  current->emitBytes(OP_METHOD, constant);
}

static void classDeclaration()
{
  current->parser->consume(TokenType::IDENTIFIER, "Expect class name.");
  Token className = current->parser->previous();
  uint8_t nameconstant =
      current->identifierConstant(current->parser->previous());
  current->declareVariable();

  current->emitBytes(OP_CLASS, nameconstant);
  current->defineVariable(nameconstant);

  ClassCompiler classCompiler;
  classCompiler.enclosing = currentClass;
  currentClass = &classCompiler;

  if (current->parser->match(TokenType::LESS)) {
    current->parser->consume(TokenType::IDENTIFIER, "Expect superclass name.");
    variable(false);

    if (identifierEqual(className, current->parser->previous())) {
      current->parser->error("A class can't inherit from itself.");
    }

    current->beginScope();
    current->addLocal(syntheticToken("super"));
    current->defineVariable(0);

    namedVariable(className, false);
    current->emitByte(OP_INHERIT);
    classCompiler.hasSuperclass = true;
  }

  namedVariable(className, false);

  current->parser->consume(TokenType::LEFT_BRACE,
                           "Expect '{' before class body.");

  while (!current->parser->check(TokenType::RIGHT_BRACE)
         && !current->parser->check(TokenType::END_OF_FILE))
  {
    method();
  }

  current->parser->consume(TokenType::RIGHT_BRACE,
                           "Expect '}' after class body.");
  current->emitByte(OP_POP);

  if (classCompiler.hasSuperclass) {
    current->endScope();
  }

  currentClass = currentClass->enclosing;
}

static void binary(bool)
{
  TokenType opType = current->parser->previous().type();
  ParseRule rule = getRule(opType);

  // TODO: Change this: define + operator for Precedence
  current->parsePrecedence((Precedence)(static_cast<int>(rule.precedence) + 1));

  switch (opType) {
    case TokenType::BANG_EQUAL:
      current->emitBytes(OP_EQUAL, OP_NOT);
      break;
    case TokenType::EQUAL_EQUAL:
      current->emitByte(OP_EQUAL);
      break;
    case TokenType::GREATER:
      current->emitByte(OP_GREATER);
      break;
    case TokenType::GREATER_EQUAL:
      current->emitBytes(OP_LESS, OP_NOT);
      break;
    case TokenType::LESS:
      current->emitByte(OP_LESS);
      break;
    case TokenType::LESS_EQUAL:
      current->emitBytes(OP_GREATER, OP_NOT);
      break;
    case TokenType::PLUS:
      current->emitByte(OP_ADD);
      break;
    case TokenType::MINUS:
      current->emitByte(OP_SUBTRACT);
      break;
    case TokenType::STAR:
      current->emitByte(OP_MULTIPLY);
      break;
    case TokenType::SLASH:
      current->emitByte(OP_DIVIDE);
      break;
    default:
      return;  // Unreachable
  }
}

static void unary(bool)
{
  TokenType operatorType = current->parser->previous().type();

  // compile operand
  current->parsePrecedence(Precedence::UNARY);

  switch (operatorType) {
    case TokenType::BANG:
      current->emitByte(OP_NOT);
      break;
    case TokenType::MINUS:
      current->emitByte(OP_NEGATE);
      break;
    default:
      return;  // Unreachable
  }
}

static uint8_t argumentList()
{
  uint8_t argCount = 0;

  if (!current->parser->check(TokenType::RIGHT_PAREN)) {
    do {
      expression();

      if (argCount == 255) {
        current->parser->error("Can't have more than 255 arguments.");
      }

      argCount++;
    } while (current->parser->match(TokenType::COMMA));
  }

  current->parser->consume(TokenType::RIGHT_PAREN,
                           "Expect ')' after arguments.");
  return argCount;
}

static void call(bool)
{
  uint8_t argCount = argumentList();
  current->emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign)
{
  current->parser->consume(TokenType::IDENTIFIER,
                           "Expect property name after '.'.");
  uint8_t name = current->identifierConstant(current->parser->previous());

  if (canAssign && current->parser->match(TokenType::EQUAL)) {
    expression();
    current->emitBytes(OP_SET_PROPERTY, name);
  } else if (current->parser->match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    current->emitBytes(OP_INVOKE, name);
    current->emitByte(argCount);
  } else {
    current->emitBytes(OP_GET_PROPERTY, name);
  }
}

static void literal(bool)
{
  switch (current->parser->previous().type()) {
    case TokenType::FALSE:
      current->emitByte(OP_FALSE);
      break;
    case TokenType::NIL:
      current->emitByte(OP_NIL);
      break;
    case TokenType::TRUE:
      current->emitByte(OP_TRUE);
      break;
    default:
      return;  // Unreachable
  }
}

static void funDeclaration()
{
  uint8_t global = current->parseVariable("Expect function name.");
  current->markInitialized();
  function(FunctionType::FUNCTION);
  current->defineVariable(global);
}

static void varDeclaration()
{
  uint8_t global = current->parseVariable("Expect variable name.");

  if (current->parser->match(TokenType::EQUAL)) {
    expression();
  } else {
    current->emitByte(OP_NIL);
  }

  current->parser->consume(TokenType::SEMICOLON,
                           "Expect ';' after variable declaration.");
  current->defineVariable(global);
}

static void expressionStatement()
{
  expression();
  current->parser->consume(TokenType::SEMICOLON,
                           "Expect ';' after expression.");
  current->emitByte(OP_POP);
}

static void forStatement()
{
  current->beginScope();

  current->parser->consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
  if (current->parser->match(TokenType::SEMICOLON)) {
    // no initializer -> do nothing
  } else if (current->parser->match(TokenType::VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }

  int loopStart = current->currentChunk()->count();
  int exitJump = -1;

  if (!current->parser->match(TokenType::SEMICOLON)) {
    expression();
    current->parser->consume(TokenType::SEMICOLON,
                             "Expect ';' after loop condition.");

    // Jump out of the loop if the condition is false
    exitJump = current->emitJump(OP_JUMP_IF_FALSE);
    current->emitByte(OP_POP);
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

  if (!current->parser->match(TokenType::RIGHT_PAREN)) {
    int bodyJump = current->emitJump(
        OP_JUMP);  // unconditionally jump over increment clause
    int incrementStart = current->currentChunk()->count();
    expression();  // compile increment clause
    current->emitByte(OP_POP);  // expression only executed for sideeffect, pop
                                // value off stack

    current->parser->consume(TokenType::RIGHT_PAREN,
                             "Expect ')' after for clauses.");

    current->emitLoop(loopStart);
    loopStart = incrementStart;
    current->patchJump(bodyJump);
  }

  statement();
  current->emitLoop(loopStart);

  if (exitJump != -1) {
    current->patchJump(exitJump);
    current->emitByte(OP_POP);
  }

  current->endScope();
}

static void ifStatement()
{
  current->parser->consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  current->parser->consume(TokenType::RIGHT_PAREN,
                           "Expect ')' after condition.");

  const int thenJump = current->emitJump(OP_JUMP_IF_FALSE);
  current->emitByte(OP_POP);
  statement();

  const int elseJump = current->emitJump(OP_JUMP);

  current->patchJump(thenJump);

  current->emitByte(OP_POP);

  if (current->parser->match(TokenType::ELSE)) {
    statement();
  }

  current->patchJump(elseJump);
}

static void whileStatement()
{
  int loopStart = current->currentChunk()->count();

  current->parser->consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  current->parser->consume(TokenType::RIGHT_PAREN,
                           "Expect ')' after condition.");
  int exitJump = current->emitJump(OP_JUMP_IF_FALSE);
  current->emitByte(OP_POP);
  statement();

  current->emitLoop(loopStart);

  current->patchJump(exitJump);
  current->emitByte(OP_POP);
}

static void printStatement()
{
  expression();
  current->parser->consume(TokenType::SEMICOLON, "Expect ';' after value.");
  current->emitByte(OP_PRINT);
}

static void returnStatement()
{
  if (current->type == FunctionType::SCRIPT) {
    current->parser->error("Can't return from top-level code.");
  }

  if (current->parser->match(TokenType::SEMICOLON)) {
    current->emitReturn();
  } else {
    if (current->type == FunctionType::INITIALIZER) {
      current->parser->error("Can't return a value from an initializer.");
    }

    expression();
    current->parser->consume(TokenType::SEMICOLON,
                             "Expect ';' after return value.");
    current->emitByte(OP_RETURN);
  }
}

static void statement()
{
  if (current->parser->match(TokenType::PRINT)) {
    printStatement();
  } else if (current->parser->match(TokenType::FOR)) {
    forStatement();
  } else if (current->parser->match(TokenType::IF)) {
    ifStatement();
  } else if (current->parser->match(TokenType::RETURN)) {
    returnStatement();
  } else if (current->parser->match(TokenType::WHILE)) {
    whileStatement();
  } else if (current->parser->match(TokenType::LEFT_BRACE)) {
    current->beginScope();
    block();
    current->endScope();
  } else {
    expressionStatement();
  }
}

static void declaration()
{
  if (current->parser->match(TokenType::CLASS)) {
    classDeclaration();
  } else if (current->parser->match(TokenType::FUN)) {
    funDeclaration();
  } else if (current->parser->match(TokenType::VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (current->parser->panicMode()) {
    current->parser->synchronize();
  }
}

static void and_(bool)
{
  int endJump = current->emitJump(OP_JUMP_IF_FALSE);

  current->emitByte(OP_POP);
  current->parsePrecedence(Precedence::AND);
  current->patchJump(endJump);
}

static void or_(bool)
{
  int elseJump = current->emitJump(OP_JUMP_IF_FALSE);
  int endJump = current->emitJump(OP_JUMP);

  current->patchJump(elseJump);
  current->emitByte(OP_POP);

  current->parsePrecedence(Precedence::OR);
  current->patchJump(endJump);
}

static void super_(bool)
{
  if (currentClass == nullptr) {
    current->parser->error("Can't use 'super' outside of a class.");
  } else if (!currentClass->hasSuperclass) {
    current->parser->error("Can't use 'super' in a class with no superclass.");
  }

  current->parser->consume(TokenType::DOT, "Expect '.' after 'super'.");
  current->parser->consume(TokenType::IDENTIFIER,
                           "Expect superclass method name.");
  uint8_t name = current->identifierConstant(current->parser->previous());

  namedVariable(syntheticToken("this"), false);
  if (current->parser->match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    namedVariable(syntheticToken("super"), false);
    current->emitBytes(OP_SUPER_INVOKE, name);
    current->emitByte(argCount);
  } else {
    namedVariable(syntheticToken("super"), false);
    current->emitBytes(OP_GET_SUPER, name);
  }
}

static void grouping(bool)
{
  expression();
  current->parser->consume(TokenType::RIGHT_PAREN,
                           "Expect ')' after expression.");
}

static void number(bool)
{
  double value =
      std::stod(std::string {current->parser->previous().string()}, nullptr);

  current->emitConstant(Value(value));
}

static void string_(bool)
{
  auto str = current->parser->previous().string();

  current->emitConstant(Value(current->mm->copyString(
      std::string_view {str.data() + 1, str.length() - 2})));
}

static void this_(bool)
{
  if (currentClass == nullptr) {
    current->parser->error("Can't use 'this' outside of a class.");
    return;
  }

  variable(false);
}

static ParseRule getRule(TokenType type)
{
  switch (type) {
    case TokenType::LEFT_PAREN:
      return {grouping, call, Precedence::CALL};

    case TokenType::DOT:
      return {nullptr, dot, Precedence::CALL};

    case TokenType::MINUS:
      return {unary, binary, Precedence::TERM};

    case TokenType::PLUS:
      return {nullptr, binary, Precedence::TERM};

    case TokenType::SLASH:
    case TokenType::STAR:
      return {nullptr, binary, Precedence::FACTOR};

    case TokenType::BANG:
      return {unary, nullptr, Precedence::NONE};

    case TokenType::BANG_EQUAL:
    case TokenType::EQUAL_EQUAL:
      return {nullptr, binary, Precedence::EQUALITY};

    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
      return {nullptr, binary, Precedence::COMPARISON};
    case TokenType::AND:
      return {nullptr, and_, Precedence::AND};
    case TokenType::OR:
      return {nullptr, or_, Precedence::OR};

    case TokenType::SUPER:
      return {super_, nullptr, Precedence::NONE};

    case TokenType::THIS:
      return {this_, nullptr, Precedence::NONE};

    case TokenType::NIL:
    case TokenType::FALSE:
    case TokenType::TRUE:
      return {literal, nullptr, Precedence::NONE};

    case TokenType::STRING:
      return {string_, nullptr, Precedence::NONE};

    case TokenType::NUMBER:
      return {number, nullptr, Precedence::NONE};

    case TokenType::IDENTIFIER:
      return {variable, nullptr, Precedence::NONE};

    case TokenType::RIGHT_PAREN:
    case TokenType::LEFT_BRACE:
    case TokenType::RIGHT_BRACE:
    case TokenType::COMMA:
    case TokenType::SEMICOLON:
    case TokenType::EQUAL:
    case TokenType::CLASS:
    case TokenType::ELSE:
    case TokenType::FOR:
    case TokenType::FUN:
    case TokenType::IF:
    case TokenType::RETURN:
    case TokenType::VAR:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::ERROR:
    case TokenType::END_OF_FILE:
      return {nullptr, nullptr, Precedence::NONE};
  }

  assert(false);
  return {nullptr, nullptr, Precedence::NONE};
}

ObjFunction* compile(MemoryManager* mm, std::string_view source)
{
  Scanner scanner {source};
  Parser parser {scanner};

  Compiler compiler;
  initCompiler(&compiler, mm, &parser, FunctionType::SCRIPT);

  current->parser->advance();

  while (!current->parser->match(TokenType::END_OF_FILE)) {
    declaration();
  }

  auto function = endCompiler();
  return parser.hadError() ? nullptr : function;
}

void markCompilerRoots()
{
  Compiler* compiler = current;
  while (compiler != nullptr) {
    compiler->mm->markObject(compiler->function);
    compiler = compiler->enclosing;
  }
}

Compiler::Compiler(Compiler* enclosing,
                   MemoryManager* memory_manager,
                   Parser* parser,
                   FunctionType type)
{
  this->enclosing = enclosing;

  mm = memory_manager;
  this->parser = parser;

  function = nullptr;
  this->type = type;
  localCount = 0;
  scopeDepth = 0;
  function = mm->newFunction();

  if (type != FunctionType::SCRIPT) {
    function->setName(mm->copyString(parser->previous().string()));
  }

  Local* local = &locals[localCount++];
  local->depth = 0;
  local->isCaptured = false;

  if (type != FunctionType::FUNCTION) {
    local->name = Token {TokenType::IDENTIFIER, "this", 0};
  } else {
    local->name = Token {TokenType::IDENTIFIER, "", 0};
  }
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

int Compiler::resolveLocal(Compiler* compiler, Token name)
{
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifierEqual(name, local->name)) {
      if (local->depth == -1) {
        parser->error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

uint8_t Compiler::identifierConstant(Token name)
{
  return makeConstant(Value(mm->copyString(name.string())));
}

int Compiler::addUpvalue(Compiler* compiler, uint8_t index, bool isLocal)
{
  int upvalueCount = compiler->function->upvalueCount();

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if (upvalueCount == UINT8_COUNT) {
    parser->error("Too many closure variables in function.");
    return 0;
  }

  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;
  const auto tmp = compiler->function->upvalueCount();
  compiler->function->incrementUpvalueCount();
  return tmp;
}

int Compiler::resolveUpvalue(Compiler* compiler, Token name)
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

void Compiler::addLocal(Token name)
{
  if (localCount == UINT8_COUNT) {
    parser->error("Too many local variables in function.");
    return;
  }

  Local* local = &locals[localCount++];
  local->name = name;
  local->depth = -1;
  local->isCaptured = false;
}

void Compiler::declareVariable()
{
  if (scopeDepth == 0) {
    return;
  }

  Token name = parser->previous();

  for (int i = localCount - 1; i >= 0; i--) {
    Local* local = &locals[i];
    if (local->depth != -1 && local->depth < scopeDepth) {
      break;
    }

    if (identifierEqual(name, local->name)) {
      parser->error("Already a variable with this name in this scope.");
    }
  }

  addLocal(name);
}

void Compiler::defineVariable(uint8_t global)
{
  if (scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

uint8_t Compiler::parseVariable(const char* errorMessage)
{
  parser->consume(TokenType::IDENTIFIER, errorMessage);

  declareVariable();
  if (scopeDepth > 0) {
    return 0;
  }

  return identifierConstant(parser->previous());
}

void Compiler::parsePrecedence(Precedence precedence)
{
  parser->advance();
  ParseFn prefixRule = getRule(parser->previous().type()).prefix;
  if (prefixRule == nullptr) {
    parser->error("Expect expression.");
    return;
  }

  const bool canAssign = precedence <= Precedence::ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser->current().type()).precedence) {
    parser->advance();
    ParseFn infixRule = getRule(parser->previous().type()).infix;
    infixRule(canAssign);
  }

  // invalid target for an assignment leads to the = not being consumed
  // Example: a * b = c + d;
  // this ensures an error is emitted
  if (canAssign && parser->match(TokenType::EQUAL)) {
    parser->error("Invalid assignment target.");
  }
}

void Compiler::markInitialized()
{
  if (scopeDepth == 0) {
    return;  // functions are marked initialised too and may appear in
             // non-local scopes
  }

  locals[localCount - 1].depth = scopeDepth;
}

void Compiler::patchJump(int offset)
{
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentChunk()->count() - offset - 2;

  if (jump > UINT16_MAX) {
    parser->error("Too much code to jump over.");
  }

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

void Compiler::emitConstant(Value value)
{
  emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value)
{
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    parser->error("Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)constant;
}

void Compiler::emitLoop(int loopStart)
{
  emitByte(OP_LOOP);

  int offset = currentChunk()->count() - loopStart + 2;
  if (offset > UINT16_MAX) {
    parser->error("Loop body too large.");
  }

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

void Compiler::emitReturn()
{
  if (type == FunctionType::INITIALIZER) {
    emitBytes(OP_GET_LOCAL, 0);
  } else {
    emitByte(OP_NIL);  // functions without a return value implicitely return
                       // nil, except for initializers
  }

  emitByte(OP_RETURN);
}

int Compiler::emitJump(uint8_t instruction)
{
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count() - 2;
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
  emitByte(byte1);
  emitByte(byte2);
}

void Compiler::emitByte(uint8_t byte)
{
  currentChunk()->write(byte, parser->previous().line());
}

Chunk* Compiler::currentChunk()
{
  return function->chunk();
}

ObjFunction* Compiler::endCompiler()
{
  emitReturn();
  ObjFunction* f = function;

#ifdef DEBUG_PRINT_CODE
  if (!parser->hadError()) {
    disassembleChunk(currentChunk(),
                     f->name() != nullptr ? f->name()->toString() : "<script>");
  }
#endif

  return f;
}
