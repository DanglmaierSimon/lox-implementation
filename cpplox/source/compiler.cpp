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

Compiler::Compiler(Compiler* enclosing,
                   MemoryManager* memory_manager,
                   Parser parser,
                   FunctionType type)
    : _enclosing(enclosing)
    , _function(nullptr)
    , scopeDepth(0)
    , localCount(0)
    , type(type)
    , _mm(memory_manager)
    , parser(parser)
{
  memoryManager()->setCurrentCompiler(this);
  _function = memoryManager()->newFunction();

  if (type != FunctionType::SCRIPT) {
    function()->setName(
        memoryManager()->copyString(parser.previous().string()));
  }

  if (type != FunctionType::FUNCTION) {
    locals[0].name = Token {TokenType::IDENTIFIER, "this", 0};
  } else {
    locals[0].name = Token {TokenType::IDENTIFIER, "", 0};
  }

  localCount++;

  if (enclosing != nullptr) {
    _currentClass = enclosing->_currentClass;
  }
}

Compiler::~Compiler()
{
  if (enclosing() != nullptr) {
    enclosing()->parser = parser;
    memoryManager()->setCurrentCompiler(enclosing());
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

int Compiler::resolveLocal(Token name)
{
  for (int i = localCount - 1; i >= 0; i--) {
    Local local = locals[i];
    if (identifierEqual(name, local.name)) {
      if (local.depth == -1) {
        parser.error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

uint8_t Compiler::identifierConstant(Token name)
{
  return makeConstant(Value(memoryManager()->copyString(name.string())));
}

int Compiler::addUpvalue(uint8_t index, bool isLocal)
{
  int upvalueCount = function()->upvalueCount();

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue* upvalue = &upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if (upvalueCount == UINT8_COUNT) {
    parser.error("Too many closure variables in function.");
    return 0;
  }

  upvalues[upvalueCount].isLocal = isLocal;
  upvalues[upvalueCount].index = index;
  const auto tmp = function()->upvalueCount();
  function()->incrementUpvalueCount();
  return tmp;
}

int Compiler::resolveUpvalue(Token name)
{
  if (enclosing() == nullptr) {
    return -1;
  }

  int local = enclosing()->resolveLocal(name);

  if (local != -1) {
    enclosing()->locals[local].isCaptured = true;
    return addUpvalue((uint8_t)local, true);
  }

  int upvalue = enclosing()->resolveUpvalue(name);
  if (upvalue != -1) {
    return addUpvalue((uint8_t)upvalue, false);
  }

  return -1;
}

void Compiler::addLocal(Token name)
{
  if (localCount == UINT8_COUNT) {
    parser.error("Too many local variables in function.");
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

  Token name = parser.previous();

  for (int i = localCount - 1; i >= 0; i--) {
    Local* local = &locals[i];
    if (local->depth != -1 && local->depth < scopeDepth) {
      break;
    }

    if (identifierEqual(name, local->name)) {
      parser.error("Already a variable with this name in this scope.");
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
  parser.consume(TokenType::IDENTIFIER, errorMessage);

  declareVariable();
  if (scopeDepth > 0) {
    return 0;
  }

  return identifierConstant(parser.previous());
}

void Compiler::parsePrecedence(Precedence precedence)
{
  parser.advance();
  ParseFn prefixRule = getRule(parser.previous().type()).prefix;
  if (prefixRule == nullptr) {
    parser.error("Expect expression.");
    return;
  }

  const bool canAssign = precedence <= Precedence::ASSIGNMENT;
  (this->*prefixRule)(canAssign);

  while (precedence <= getRule(parser.current().type()).precedence) {
    parser.advance();
    ParseFn infixRule = getRule(parser.previous().type()).infix;
    (this->*infixRule)(canAssign);
  }

  // invalid target for an assignment leads to the = not being consumed
  // Example: a * b = c + d;
  // this ensures an error is emitted
  if (canAssign && parser.match(TokenType::EQUAL)) {
    parser.error("Invalid assignment target.");
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

void Compiler::patchJump(size_t offset)
{
  assert(currentChunk()->count() >= (offset - 2));
  // -2 to adjust for the bytecode for the jump offset itself.
  size_t jump = currentChunk()->count() - offset - 2;

  if (jump > UINT16_MAX) {
    parser.error("Too much code to jump over.");
  }

  currentChunk()->writeAt(offset, (jump >> 8) & 0xff);
  currentChunk()->writeAt(offset + 1, jump & 0xff);
}

void Compiler::emitConstant(Value value)
{
  emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value)
{
  auto constant = currentChunk()->addConstant(value);
  if (constant > UINT8_MAX) {
    parser.error("Too many constants in one chunk.");
    return 0;
  }

  return static_cast<uint8_t>(constant);
}

void Compiler::emitLoop(int loopStart)
{
  emitByte(OP_LOOP);

  int offset = currentChunk()->count() - loopStart + 2;
  if (offset > UINT16_MAX) {
    parser.error("Loop body too large.");
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
  currentChunk()->write(byte, parser.previous().line());
}

Chunk* Compiler::currentChunk()
{
  return function()->chunk();
}

ObjFunction* Compiler::endCompiler()
{
  emitReturn();
  ObjFunction* f = function();

#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError()) {
    disassembleChunk(currentChunk(),
                     f->name() != nullptr ? f->name()->toString() : "<script>");
  }
#endif

  return f;
}

ObjFunction* Compiler::compileFunction()
{
  beginScope();

  parser.consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

  if (!parser.check(TokenType::RIGHT_PAREN)) {
    do {
      function()->incrementArity();
      if (function()->arity() > 255) {
        parser.errorAtCurrent("Can't have more than 255 parameters.");
      }

      uint8_t constant = parseVariable("Expect parameter name.");
      defineVariable(constant);
    } while (parser.match(TokenType::COMMA));
  }

  parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
  parser.consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
  block();

  return endCompiler();
}

void Compiler::grouping(bool)
{
  expression();
  parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

ParseRule Compiler::getRule(TokenType t)
{
  switch (t) {
    case TokenType::LEFT_PAREN:
      return {&Compiler::grouping, &Compiler::call, Precedence::CALL};

    case TokenType::DOT:
      return {nullptr, &Compiler::dot, Precedence::CALL};

    case TokenType::MINUS:
      return {&Compiler::unary, &Compiler::binary, Precedence::TERM};

    case TokenType::PLUS:
      return {nullptr, &Compiler::binary, Precedence::TERM};

    case TokenType::SLASH:
    case TokenType::STAR:
      return {nullptr, &Compiler::binary, Precedence::FACTOR};

    case TokenType::BANG:
      return {&Compiler::unary, nullptr, Precedence::NONE};

    case TokenType::BANG_EQUAL:
    case TokenType::EQUAL_EQUAL:
      return {nullptr, &Compiler::binary, Precedence::EQUALITY};

    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
      return {nullptr, &Compiler::binary, Precedence::COMPARISON};
    case TokenType::AND:
      return {nullptr, &Compiler::and_, Precedence::AND};
    case TokenType::OR:
      return {nullptr, &Compiler::or_, Precedence::OR};

    case TokenType::SUPER:
      return {&Compiler::super_, nullptr, Precedence::NONE};

    case TokenType::THIS:
      return {&Compiler::this_, nullptr, Precedence::NONE};

    case TokenType::NIL:
    case TokenType::FALSE:
    case TokenType::TRUE:
      return {&Compiler::literal, nullptr, Precedence::NONE};

    case TokenType::STRING:
      return {&Compiler::string_, nullptr, Precedence::NONE};

    case TokenType::NUMBER:
      return {&Compiler::number, nullptr, Precedence::NONE};

    case TokenType::IDENTIFIER:
      return {&Compiler::variable, nullptr, Precedence::NONE};

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

void Compiler::expression()
{
  parsePrecedence(Precedence::ASSIGNMENT);
}

void Compiler::namedVariable(Token name, bool canAssign)
{
  uint8_t getOp, setOp;
  auto arg = resolveLocal(name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((arg = resolveUpvalue(name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  } else {
    arg = identifierConstant(name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && parser.match(TokenType::EQUAL)) {
    expression();
    emitBytes(setOp, static_cast<uint8_t>(arg));
  } else {
    emitBytes(getOp, static_cast<uint8_t>(arg));
  }
}

void Compiler::variable(bool canAssign)
{
  namedVariable(parser.previous(), canAssign);
}

void Compiler::block()
{
  while (!parser.check(TokenType::RIGHT_BRACE)
         && !parser.check(TokenType::END_OF_FILE))
  {
    declaration();
  }

  parser.consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::function_(FunctionType t)
{
  Compiler functionCompiler {this, memoryManager(), parser, t};

  auto f = functionCompiler.compileFunction();

  emitBytes(OP_CLOSURE, makeConstant(Value(f)));

  for (int i = 0; i < f->upvalueCount(); i++) {
    emitByte(functionCompiler.upvalues[i].isLocal ? 1 : 0);
    emitByte(functionCompiler.upvalues[i].index);
  }
}

void Compiler::method()
{
  parser.consume(TokenType::IDENTIFIER, "Expect method name.");
  uint8_t constant = identifierConstant(parser.previous());

  FunctionType t = FunctionType::METHOD;

  if (parser.previous().string() == "init") {
    t = FunctionType::INITIALIZER;
  }

  function_(t);

  emitBytes(OP_METHOD, constant);
}

void Compiler::classDeclaration()
{
  parser.consume(TokenType::IDENTIFIER, "Expect class name.");
  Token className = parser.previous();
  uint8_t nameconstant = identifierConstant(parser.previous());
  declareVariable();

  emitBytes(OP_CLASS, nameconstant);
  defineVariable(nameconstant);

  ClassCompiler classCompiler;
  classCompiler.enclosing = _currentClass;
  _currentClass = &classCompiler;

  if (parser.match(TokenType::LESS)) {
    parser.consume(TokenType::IDENTIFIER, "Expect superclass name.");
    variable(false);

    if (identifierEqual(className, parser.previous())) {
      parser.error("A class can't inherit from itself.");
    }

    beginScope();
    addLocal(syntheticToken("super"));
    defineVariable(0);

    namedVariable(className, false);
    emitByte(OP_INHERIT);
    classCompiler.hasSuperclass = true;
  }

  namedVariable(className, false);

  parser.consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

  while (!parser.check(TokenType::RIGHT_BRACE)
         && !parser.check(TokenType::END_OF_FILE))
  {
    method();
  }

  parser.consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
  emitByte(OP_POP);

  if (classCompiler.hasSuperclass) {
    endScope();
  }

  _currentClass = _currentClass->enclosing;
}

void Compiler::binary(bool)
{
  TokenType opType = parser.previous().type();
  ParseRule rule = getRule(opType);

  // TODO: Change this: define + operator for Precedence
  parsePrecedence((Precedence)(static_cast<int>(rule.precedence) + 1));

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

void Compiler::unary(bool)
{
  TokenType operatorType = parser.previous().type();

  // compile operand
  parsePrecedence(Precedence::UNARY);

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

uint8_t Compiler::argumentList()
{
  uint8_t argCount = 0;

  if (!parser.check(TokenType::RIGHT_PAREN)) {
    do {
      expression();

      if (argCount == 255) {
        parser.error("Can't have more than 255 arguments.");
      }

      argCount++;
    } while (parser.match(TokenType::COMMA));
  }

  parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}

void Compiler::call(bool)
{
  uint8_t argCount = argumentList();
  emitBytes(OP_CALL, argCount);
}

void Compiler::dot(bool canAssign)
{
  parser.consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
  uint8_t name = identifierConstant(parser.previous());

  if (canAssign && parser.match(TokenType::EQUAL)) {
    expression();
    emitBytes(OP_SET_PROPERTY, name);
  } else if (parser.match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    emitBytes(OP_INVOKE, name);
    emitByte(argCount);
  } else {
    emitBytes(OP_GET_PROPERTY, name);
  }
}

void Compiler::literal(bool)
{
  switch (parser.previous().type()) {
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

void Compiler::funDeclaration()
{
  uint8_t global = parseVariable("Expect function name.");
  markInitialized();
  function_(FunctionType::FUNCTION);
  defineVariable(global);
}

void Compiler::super_(bool)
{
  if (_currentClass == nullptr) {
    parser.error("Can't use 'super' outside of a class.");
  } else if (!_currentClass->hasSuperclass) {
    parser.error("Can't use 'super' in a class with no superclass.");
  }

  parser.consume(TokenType::DOT, "Expect '.' after 'super'.");
  parser.consume(TokenType::IDENTIFIER, "Expect superclass method name.");
  uint8_t name = identifierConstant(parser.previous());

  namedVariable(syntheticToken("this"), false);
  if (parser.match(TokenType::LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_SUPER_INVOKE, name);
    emitByte(argCount);
  } else {
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_GET_SUPER, name);
  }
}

void Compiler::number(bool)
{
  double value = std::stod(std::string {parser.previous().string()}, nullptr);

  emitConstant(Value(value));
}

void Compiler::string_(bool)
{
  auto str = parser.previous().string();

  emitConstant(Value(memoryManager()->copyString(
      std::string_view {str.data() + 1, str.length() - 2})));
}

void Compiler::this_(bool)
{
  if (_currentClass == nullptr) {
    parser.error("Can't use 'this' outside of a class.");
    return;
  }

  variable(false);
}

void Compiler::or_(bool)
{
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(Precedence::OR);
  patchJump(endJump);
}

void Compiler::and_(bool)
{
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(Precedence::AND);
  patchJump(endJump);
}

void Compiler::varDeclaration()
{
  uint8_t global = parseVariable("Expect variable name.");

  if (parser.match(TokenType::EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }

  parser.consume(TokenType::SEMICOLON,
                 "Expect ';' after variable declaration.");
  defineVariable(global);
}

void Compiler::expressionStatement()
{
  expression();
  parser.consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Compiler::forStatement()
{
  beginScope();

  parser.consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
  if (parser.match(TokenType::SEMICOLON)) {
    // no initializer -> do nothing
  } else if (parser.match(TokenType::VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }

  int loopStart = currentChunk()->count();
  int exitJump = -1;

  if (!parser.match(TokenType::SEMICOLON)) {
    expression();
    parser.consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

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

  if (!parser.match(TokenType::RIGHT_PAREN)) {
    int bodyJump =
        emitJump(OP_JUMP);  // unconditionally jump over increment clause
    int incrementStart = currentChunk()->count();
    expression();  // compile increment clause
    emitByte(OP_POP);  // expression only executed for sideeffect,
                       // pop value off stack

    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

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
  parser.consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

  const int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  const int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);

  emitByte(OP_POP);

  if (parser.match(TokenType::ELSE)) {
    statement();
  }

  patchJump(elseJump);
}

void Compiler::whileStatement()
{
  int loopStart = currentChunk()->count();

  parser.consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
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
  parser.consume(TokenType::SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

void Compiler::returnStatement()
{
  if (type == FunctionType::SCRIPT) {
    parser.error("Can't return from top-level code.");
  }

  if (parser.match(TokenType::SEMICOLON)) {
    emitReturn();
  } else {
    if (type == FunctionType::INITIALIZER) {
      parser.error("Can't return a value from an initializer.");
    }

    expression();
    parser.consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    emitByte(OP_RETURN);
  }
}

void Compiler::statement()
{
  if (parser.match(TokenType::PRINT)) {
    printStatement();
  } else if (parser.match(TokenType::FOR)) {
    forStatement();
  } else if (parser.match(TokenType::IF)) {
    ifStatement();
  } else if (parser.match(TokenType::RETURN)) {
    returnStatement();
  } else if (parser.match(TokenType::WHILE)) {
    whileStatement();
  } else if (parser.match(TokenType::LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

void Compiler::declaration()
{
  if (parser.match(TokenType::CLASS)) {
    classDeclaration();
  } else if (parser.match(TokenType::FUN)) {
    funDeclaration();
  } else if (parser.match(TokenType::VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode()) {
    parser.synchronize();
  }
}

ObjFunction* Compiler::compile()
{
  parser.advance();

  while (!parser.match(TokenType::END_OF_FILE)) {
    declaration();
  }

  auto f = endCompiler();
  return parser.hadError() ? nullptr : f;
}

MemoryManager* Compiler::memoryManager()
{
  return _mm;
}

Compiler* Compiler::enclosing()
{
  return _enclosing;
}

ObjFunction* Compiler::function()
{
  return _function;
}
