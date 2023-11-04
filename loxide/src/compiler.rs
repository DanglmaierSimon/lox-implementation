use std::{collections::HashMap, rc::Rc};

use crate::{
    chunk::Chunk,
    debug::disassemble_chunk,
    gc::MemoryManager,
    object::{copy_string, Obj},
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

type ParseFn = fn(&mut CParams);

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
        });
    }
}

// Parameters used during compilation, passed to every function
struct CParams<'a> {
    chunk: Chunk,
    gc: &'a MemoryManager,
    scanner: Scanner,
    parser: Parser,
}

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

pub fn compile(scanner: Scanner, gc: &MemoryManager) -> (bool, Chunk) {
    let parser = Parser::new();
    let chunk = Chunk::new();

    let mut params = CParams {
        scanner,
        parser,
        chunk,
        gc,
    };

    advance(&mut params);

    while !cmatch(&mut params, TokenType::EOF) {
        declaration(&mut params);
    }

    end_compiler(&mut params);

    return (!params.parser.had_error, params.chunk);
}

fn declaration(params: &mut CParams) {
    if cmatch(params, TokenType::VAR) {
        var_declaration(params);
    } else {
        statement(params);
    }

    if params.parser.panic_mode {
        synchronize(params);
    }
}

fn var_declaration(params: &mut CParams) {
    // var a;
    // and
    // var a = <expression>;
    // are both valid forms

    let global: usize = parse_variable(params, "Expect variable name.");

    if cmatch(params, TokenType::EQUAL) {
        expression(params);
    } else {
        // if no expression is found, the variable declaration is de-sugared into var a = nil;
        emit_byte(params, OpCode::Nil);
    }

    consume(
        params,
        TokenType::SEMICOLON,
        "Expect ';' after variable declaration.",
    );

    define_variable(params, global);
}

fn define_variable(params: &mut CParams, global: usize) {
    emit_byte(params, OpCode::DefineGlobal(global))
}

fn parse_variable(params: &mut CParams, error_msg: &str) -> usize {
    consume(params, TokenType::IDENTIFIER, error_msg);
    return identifier_constant(params);
}

fn identifier_constant(params: &mut CParams) -> usize {
    return make_constant(
        params,
        Value::Obj(Box::new(Obj::String(copy_string(
            params.gc,
            params.parser.previous.string(),
        )))),
    );
}

fn synchronize(params: &mut CParams) {
    params.parser.panic_mode = false;

    while params.parser.current.token_type() != TokenType::EOF {
        if params.parser.previous.token_type() == TokenType::SEMICOLON {
            return;
        }

        match params.parser.current.token_type() {
            TokenType::CLASS
            | TokenType::FUN
            | TokenType::VAR
            | TokenType::FOR
            | TokenType::IF
            | TokenType::WHILE
            | TokenType::PRINT
            | TokenType::RETURN => return,
            _ => {
                // do nothing
            }
        }

        advance(params);
    }
}

fn statement(params: &mut CParams) {
    if cmatch(params, TokenType::PRINT) {
        print_statement(params);
    } else {
        expression_statement(params);
    }
}

fn expression_statement(params: &mut CParams) {
    expression(params);
    consume(params, TokenType::SEMICOLON, "Expect ';' after expression.");
    emit_byte(params, OpCode::Pop);
}

fn print_statement(params: &mut CParams) {
    expression(params);
    consume(params, TokenType::SEMICOLON, "Expect ';' after value.");
    emit_byte(params, OpCode::Print);
}

fn advance(params: &mut CParams) {
    params.parser.previous = params.parser.current.clone();

    loop {
        params.parser.current = params.scanner.scan_token();

        if params.parser.current.token_type() != TokenType::ERROR {
            break;
        }

        let message = params.parser.current.string().to_string();

        error_at_current(params, &message);
    }
}

fn error_at_current(params: &mut CParams, message: &str) {
    let token = params.parser.current.clone();
    error_at(params, &token, message);
}

fn error(params: &mut CParams, message: &str) {
    let prev = params.parser.previous.clone();
    error_at(params, &prev, message)
}

fn cmatch(params: &mut CParams, ttype: TokenType) -> bool {
    if !check(params, ttype) {
        return false;
    }
    advance(params);
    return true;
}

fn check(params: &mut CParams, ttype: TokenType) -> bool {
    return params.parser.current.token_type() == ttype;
}

fn error_at(params: &mut CParams, token: &Token, message: &str) {
    if params.parser.panic_mode {
        return;
    }

    params.parser.panic_mode = true;
    eprint!("[line {}] Error", token.line());

    if token.token_type() == TokenType::EOF {
        eprint!(" at end");
    } else if token.token_type() == TokenType::ERROR {
        // nothing
    } else {
        eprint!(" at '{}'", token.string());
    }

    eprintln!(": {}", message);
    params.parser.had_error = true;
}

fn consume(params: &mut CParams, ttype: TokenType, message: &str) {
    if params.parser.current.token_type() == ttype {
        advance(params);
        return;
    }

    error_at_current(params, message);
}

fn emit_byte(params: &mut CParams, opcode: OpCode) {
    let line = params.parser.previous.line();
    params.chunk.write(opcode, line);
    // current_chunk(params).write(opcode, line)
}

fn emit_bytes(params: &mut CParams, opcode1: OpCode, opcode2: OpCode) {
    emit_byte(params, opcode1);
    emit_byte(params, opcode2);
}

// fn current_chunk(params: &mut CParams) -> &mut Chunk {
//     return &mut params.chunk;
// }

fn end_compiler(params: &mut CParams) {
    emit_return(params);

    if !params.parser.had_error {
        disassemble_chunk(&params.chunk, "code");
    }
}

fn emit_return(params: &mut CParams) {
    emit_byte(params, OpCode::Return)
}

fn emit_constant(params: &mut CParams, val: Value) {
    let c = make_constant(params, val);
    emit_byte(params, OpCode::Constant(c));
}

fn make_constant(params: &mut CParams, val: Value) -> usize {
    let constant = params.chunk.add_constant(val);
    if constant > (u8::MAX as usize) {
        error(params, "Too many constants in one chunk.");
        return 0;
    }

    return constant;
}

fn parse_precedence(params: &mut CParams, precedence: Precedence) {
    advance(params);

    let prefix_rule = get_rule(params.parser.previous.token_type()).prefix;

    match prefix_rule {
        Some(rule) => {
            rule(params);
        }
        None => {
            error(params, "Expect expression.");
            return;
        }
    }

    while precedence <= get_rule(params.parser.current.token_type()).precedence {
        advance(params);

        let infix_rule = get_rule(params.parser.previous.token_type())
            .infix
            .expect("should not be null");
        infix_rule(params);
    }
}

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
            ParseRule::new(Some(unary), None, Precedence::NONE),
        ),
        (
            TokenType::BANG_EQUAL,
            ParseRule::new(None, Some(binary), Precedence::EQUALITY),
        ),
        (
            TokenType::EQUAL,
            ParseRule::new(None, None, Precedence::NONE),
        ),
        (
            TokenType::EQUAL_EQUAL,
            ParseRule::new(None, Some(binary), Precedence::EQUALITY),
        ),
        (
            TokenType::GREATER,
            ParseRule::new(None, Some(binary), Precedence::COMPARISON),
        ),
        (
            TokenType::GREATER_EQUAL,
            ParseRule::new(None, Some(binary), Precedence::COMPARISON),
        ),
        (
            TokenType::LESS,
            ParseRule::new(None, Some(binary), Precedence::COMPARISON),
        ),
        (
            TokenType::LESS_EQUAL,
            ParseRule::new(None, Some(binary), Precedence::COMPARISON),
        ),
        (
            TokenType::IDENTIFIER,
            ParseRule::new(Some(variable), None, Precedence::NONE),
        ),
        (
            TokenType::STRING,
            ParseRule::new(Some(string), None, Precedence::NONE),
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
            ParseRule::new(Some(literal), None, Precedence::NONE),
        ),
        (TokenType::FOR, ParseRule::new(None, None, Precedence::NONE)),
        (TokenType::FUN, ParseRule::new(None, None, Precedence::NONE)),
        (TokenType::IF, ParseRule::new(None, None, Precedence::NONE)),
        (
            TokenType::NIL,
            ParseRule::new(Some(literal), None, Precedence::NONE),
        ),
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
            ParseRule::new(Some(literal), None, Precedence::NONE),
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

fn binary(params: &mut CParams) {
    let operatortype = params.parser.previous.token_type();

    let rule = get_rule(operatortype);

    let prec = rule.precedence;
    parse_precedence(params, ((prec as u8) + 1).into());

    match operatortype {
        TokenType::PLUS => emit_byte(params, OpCode::Add),
        TokenType::MINUS => emit_byte(params, OpCode::Subtract),
        TokenType::STAR => emit_byte(params, OpCode::Multiply),
        TokenType::SLASH => emit_byte(params, OpCode::Divide),
        TokenType::BANG_EQUAL => emit_bytes(params, OpCode::Equal, OpCode::Not),
        TokenType::EQUAL_EQUAL => emit_byte(params, OpCode::Equal),
        TokenType::GREATER => emit_byte(params, OpCode::Greater),
        TokenType::GREATER_EQUAL => emit_bytes(params, OpCode::Less, OpCode::Not),
        TokenType::LESS => emit_byte(params, OpCode::Less),
        TokenType::LESS_EQUAL => emit_bytes(params, OpCode::Greater, OpCode::Not),
        _ => unreachable!(),
    }
}

fn grouping(params: &mut CParams) {
    expression(params);
    consume(
        params,
        TokenType::RIGHT_PAREN,
        "Expect ')' after expression.",
    );
}

fn unary(params: &mut CParams) {
    let operator_type = params.parser.previous.token_type();

    // compile operand
    parse_precedence(params, Precedence::UNARY);

    // emit operator instruction
    match operator_type {
        TokenType::MINUS => emit_byte(params, OpCode::Negate),
        TokenType::BANG => emit_byte(params, OpCode::Not),
        _ => unreachable!(),
    }
}

fn expression(params: &mut CParams) {
    parse_precedence(params, Precedence::ASSIGNMENT);
}

fn number(params: &mut CParams) {
    let val = (params.parser.previous.string())
        .parse::<f64>()
        .expect("number literal should be parseable as float");

    emit_constant(params, Value::Number(val));
}

fn literal(params: &mut CParams) {
    match params.parser.previous.token_type() {
        TokenType::FALSE => emit_byte(params, OpCode::False),
        TokenType::TRUE => emit_byte(params, OpCode::True),
        TokenType::NIL => emit_byte(params, OpCode::Nil),
        _ => unreachable!(),
    }
}

fn string(params: &mut CParams) {
    let str = copy_string(params.gc, params.parser.previous.string());

    emit_constant(params, Value::Obj(Box::new(Obj::String(str))))
}

fn variable(params: &mut CParams) {
    named_variable(params);
}

fn named_variable(params: &mut CParams) {
    let arg = identifier_constant(params);
    emit_byte(params, OpCode::GetGlobal(arg));
}
