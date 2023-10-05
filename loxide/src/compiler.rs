use std::{collections::HashMap, rc::Rc};

use crate::{
    chunk::Chunk,
    opcode::OpCode,
    scanner::Scanner,
    token::{Token, TokenType},
    value::Value,
};

#[derive(PartialEq, PartialOrd, Debug, Copy, Clone)]
#[repr(u8)]
enum Precedence {
    NONE = 0,
    ASSIGNMENT = 1, // =
    OR = 2,         // or
    AND = 3,        // and
    EQUALITY = 4,   // == !=
    COMPARISON = 5, // < > <= >=
    TERM = 6,       // + -
    FACTOR = 7,     // * /
    UNARY = 8,      // ! -
    CALL = 9,       // . ()
    PRIMARY = 10,
}

impl From<u8> for Precedence {
    fn from(value: u8) -> Self {
        match value {
            0 => return Precedence::NONE,
            1 => return Precedence::ASSIGNMENT,
            2 => return Precedence::OR,
            3 => return Precedence::AND,
            4 => return Precedence::EQUALITY,
            5 => return Precedence::COMPARISON,
            6 => return Precedence::TERM,
            7 => return Precedence::FACTOR,
            8 => return Precedence::UNARY,
            9 => return Precedence::CALL,
            10 => return Precedence::PRIMARY,
            _ => panic!("INVALID ENUM VALUE"),
        }
    }
}

type ParseFn = fn(&mut Compiler);

struct ParseRule {
    pub prefix: Option<ParseFn>,
    pub infix: Option<ParseFn>,
    pub precedence: Precedence,
}

impl ParseRule {
    fn new(prefix: Option<ParseFn>, infix: Option<ParseFn>, precedence: Precedence) -> Rc<Self> {
        return Rc::new(Self {
            prefix,
            infix,
            precedence,
        })
    }
}

struct Params {}

pub struct Parser {
    pub current: Token, // TODO
    pub previous: Token,

    pub panic_mode: bool,
    pub had_error: bool,
}

impl Parser {
    pub fn new() -> Self {
        return Self {
            current: Token::new(
                TokenType::ERROR,
                "NO TOKEN SET! YOU FUCKED UP!".to_string(),
                0,
            ),
            previous: Token::new(
                TokenType::ERROR,
                "NO TOKEN SET! YOU FUCKED UP!".to_string(),
                0,
            ),
            had_error: false,
            panic_mode: false,
        };
    }
}

pub struct Compiler {
    scanner: Scanner,
    parser: Parser,
    chunk: Chunk,
}

impl Compiler {
    pub fn new(scanner: Scanner) -> Self {
        return Self {
            scanner,
            parser: Parser::new(),
            chunk: Chunk::new(),
        };
    }
}

pub fn compile(compiler: Compiler) -> (bool, Chunk) {
    let mut compiler = compiler;
    compiler.parser.panic_mode = false;
    compiler.parser.had_error = false;

    advance(&mut compiler);
    expression(&mut compiler);
    consume(&mut compiler, TokenType::EOF, "Expect end of expression.");

    end_compiler(&mut compiler);

    return (!compiler.parser.had_error, compiler.chunk);
}

fn advance(compiler: &mut Compiler) {
    compiler.parser.previous = compiler.parser.current.clone();

    loop {
        compiler.parser.current = compiler.scanner.scan_token();

        if compiler.parser.current.token_type() != TokenType::ERROR {
            break;
        }

        let message = compiler.parser.current.string().to_string();

        error_at_current(compiler, &message);
    }
}

fn error_at_current(compiler: &mut Compiler, message: &str) {
    let token = compiler.parser.current.clone();
    error_at(compiler, &token, message);
}

fn error(compiler: &mut Compiler, message: &str) {
    let prev = compiler.parser.previous.clone();
    error_at(compiler, &prev, message)
}

fn error_at(compiler: &mut Compiler, token: &Token, message: &str) {
    if compiler.parser.panic_mode {
        return;
    }

    compiler.parser.panic_mode = true;
    eprint!("[line {}] Error", token.line());

    if token.token_type() == TokenType::EOF {
        eprint!(" at end");
    } else if token.token_type() == TokenType::ERROR {
        // nothing
    } else {
        eprint!(" at '{}'", token.string());
    }

    eprintln!(": {}", message);
    compiler.parser.had_error = true;
}

fn consume(compiler: &mut Compiler, ttype: TokenType, message: &str) {
    if compiler.parser.current.token_type() == ttype {
        advance(compiler);
        return;
    }

    error_at_current(compiler, message);
}

fn emit_byte(compiler: &mut Compiler, opcode: OpCode) {
    let line = compiler.parser.previous.line();
    current_chunk(compiler).write(opcode, line)
}

fn emit_bytes(compiler: &mut Compiler, opcode1: OpCode, opcode2: OpCode) {
    emit_byte(compiler, opcode1);
    emit_byte(compiler, opcode2);
}

fn current_chunk(compiler: &mut Compiler) -> &mut Chunk {
    return &mut compiler.chunk;
}

fn end_compiler(compiler: &mut Compiler) {
    emit_return(compiler);
}

fn emit_return(compiler: &mut Compiler) {
    emit_byte(compiler, OpCode::Return)
}

fn emit_constant(compiler: &mut Compiler, val: Value) {
    let c = make_constant(compiler, val);
    emit_byte(compiler, OpCode::Constant(c));
}

fn make_constant(compiler: &mut Compiler, val: Value) -> usize {
    let constant = current_chunk(compiler).add_constant(val);
    if constant > (u8::MAX as usize) {
        error(compiler, "Too many constants in one chunk.");
        return 0;
    }

    return constant;
}

fn parse_precedence(compiler: &mut Compiler, precedence: Precedence) {}

fn get_rule(ttype: TokenType) -> Rc<ParseRule> {
    let rules = HashMap::from([
        (
            TokenType::LEFT_PAREN,
            ParseRule::new(Some(grouping), None, Precedence::NONE),
        ),
        (
            TokenType::RIGHT_PAREN,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::LEFT_BRACE,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::RIGHT_BRACE,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::COMMA,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (TokenType::DOT, ParseRule::new(None, None, Precedence::NONE)),
        (
            TokenType::MINUS,
            ParseRule::new(Some(unary), Some(binary), Precedence::TERM),
        ),
        (
            TokenType::PLUS,
            ParseRule::new(None, Some(binary), Precedence::TERM),
        ),
        (
            TokenType::SEMICOLON,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::SLASH,
            ParseRule::new(None, Some(binary), Precedence::FACTOR),
        ),
        (
            TokenType::STAR,
            ParseRule::new(None, Some(binary), Precedence::FACTOR),
        ),
        (
            TokenType::BANG,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::BANG_EQUAL,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::EQUAL,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::EQUAL_EQUAL,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::GREATER,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::GREATER_EQUAL,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::LESS,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::LESS_EQUAL,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::IDENTIFIER,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::STRING,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::NUMBER,
            ParseRule::new(Some(number), None, Precedence::NONE),
        ),
        (TokenType::AND, ParseRule::new(None, None, Precedence::NONE)),
        (
            TokenType::CLASS,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::ELSE,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::FALSE,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (TokenType::FOR, ParseRule::new(None, None, Precedence::NONE)),
        (TokenType::FUN, ParseRule::new(None, None, Precedence::NONE)),
        (TokenType::IF, ParseRule::new(None, None, Precedence::NONE)),
        (TokenType::NIL, ParseRule::new(None, None, Precedence::NONE)),
        (TokenType::OR, ParseRule::new(None, None, Precedence::NONE)),
        (
            TokenType::PRINT,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::RETURN,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::SUPER,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::THIS,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::TRUE,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (TokenType::VAR, ParseRule::new(None, None, Precedence::NONE)),
        (
            TokenType::WHILE,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::ERROR,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (TokenType::EOF, ParseRule::new(None, None, Precedence::NONE)),
    ]);

    return Rc::clone(rules.get(&ttype).expect("UNSUPPORTED TOKENTYPE"));
}

fn binary(compiler: &mut Compiler) {
    let operatortype = compiler.parser.previous.token_type();

    let rule = get_rule(operatortype);

    let prec = rule.precedence;
    parse_precedence(compiler, ((prec as u8) + 1).into());

    match operatortype {
        TokenType::PLUS => {}
        TokenType::MINUS => {}
        TokenType::STAR => {}
        TokenType::SLASH => {}
        _ => unreachable!(),
    }
}

fn grouping(compiler: &mut Compiler) {
    expression(compiler);
    consume(
        compiler,
        TokenType::RIGHT_PAREN,
        "Expect ')' after expression.",
    );
}

fn unary(compiler: &mut Compiler) {
    let operator_type = compiler.parser.previous.token_type();

    // compile operand
    parse_precedence(compiler, Precedence::UNARY);

    // emit operator instruction
    match operator_type {
        TokenType::MINUS => emit_byte(compiler, OpCode::Negate),
        _ => unreachable!(),
    }
}

fn expression(compiler: &mut Compiler) {
    parse_precedence(compiler, Precedence::ASSIGNMENT);
}

fn number(compiler: &mut Compiler) {
    let val = (compiler.parser.previous.string())
        .parse::<f64>()
        .expect("number literal should be parseable as float");

    emit_constant(compiler, Value::Number(val));
}
