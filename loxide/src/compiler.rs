use std::collections::HashMap;

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
            11 => panic!("INVALID ENUM VALUE")
        }
    }
}

type ParseFn = fn(i32, i32) -> i32;

struct ParseRule {
    prefix:Option< ParseFn>,
    infix: Option<ParseFn>,
    precedence: Precedence,
}

impl ParseRule {
    fn new(prefix: Option<ParseFn>, infix: Option<ParseFn>, precedence: Precedence) -> Self { Self { prefix, infix, precedence } }

    
}

fn get_all_rules() -> HashMap<TokenType, ParseRule> {
    return HashMap::from([
        (TokenType::LEFT_PAREN    , ParseRule::new(grouping, None,   Precedence::NONE)),
        (TokenType::RIGHT_PAREN   , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::LEFT_BRACE    , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::RIGHT_BRACE   , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::COMMA         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::DOT           , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::MINUS         , ParseRule::new(unary,    binary, Precedence::TERM)),
        (TokenType::PLUS          , ParseRule::new(None,     binary, Precedence::TERM)),
        (TokenType::SEMICOLON     , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::SLASH         , ParseRule::new(None,     binary, Precedence::FACTOR)),
        (TokenType::STAR          , ParseRule::new(None,     binary, Precedence::FACTOR)),
        (TokenType::BANG          , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::BANG_EQUAL    , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::EQUAL         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::EQUAL_EQUAL   , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::GREATER       , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::GREATER_EQUAL , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::LESS          , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::LESS_EQUAL    , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::IDENTIFIER    , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::STRING        , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::NUMBER        , ParseRule::new(number,   None,   Precedence::NONE)),
        (TokenType::AND           , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::CLASS         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::ELSE          , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::FALSE         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::FOR           , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::FUN           , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::IF            , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::NIL           , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::OR            , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::PRINT         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::RETURN        , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::SUPER         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::THIS          , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::TRUE          , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::VAR           , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::WHILE         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::ERROR         , ParseRule::new(None,     None,   Precedence::NONE)),
        (TokenType::EOF           , ParseRule::new(None,     None,   Precedence::NONE)),



    ])
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

    pub fn compile(mut self) -> (bool, Chunk) {
        self.parser.panic_mode = false;
        self.parser.had_error = false;

        self.advance();
        self.expression();
        self.consume(TokenType::EOF, "Expect end of expression.");

        self.end_compiler();

        return (!self.parser.had_error, self.chunk);
    }

    fn advance(&mut self) {
        self.parser.previous = self.parser.current.clone();

        loop {
            self.parser.current = self.scanner.scan_token();

            if self.parser.current.token_type() != TokenType::ERROR {
                break;
            }

            let message = self.parser.current.string().to_string();

            self.error_at_current(&message);
        }
    }

    fn error_at_current(&mut self, message: &str) {
        let token = self.parser.current.clone();
        self.error_at(&token, message);
    }

    fn error(&mut self, message: &str) {
        let prev = self.parser.previous.clone();
        self.error_at(&prev, message)
    }

    fn error_at(&mut self, token: &Token, message: &str) {
        if self.parser.panic_mode {
            return;
        }

        self.parser.panic_mode = true;
        eprint!("[line {}] Error", token.line());

        if token.token_type() == TokenType::EOF {
            eprint!(" at end");
        } else if token.token_type() == TokenType::ERROR {
            // nothing
        } else {
            eprint!(" at '{}'", token.string());
        }

        eprintln!(": {}", message);
        self.parser.had_error = true;
    }

    fn consume(&mut self, ttype: TokenType, message: &str) {
        if self.parser.current.token_type() == ttype {
            self.advance();
            return;
        }

        self.error_at_current(message);
    }

    fn emit_byte(&mut self, opcode: OpCode) {
        let line = self.parser.previous.line();
        self.current_chunk().write(opcode, line)
    }

    fn emit_bytes(&mut self, opcode1: OpCode, opcode2: OpCode) {
        self.emit_byte(opcode1);
        self.emit_byte(opcode2);
    }

    fn current_chunk(&mut self) -> &mut Chunk {
        return &mut self.chunk;
    }

    fn end_compiler(&mut self) {
        self.emit_return();
    }

    fn emit_return(&mut self) {
        self.emit_byte(OpCode::Return)
    }

    fn expression(&mut self) {
        self.parse_precedence(Precedence::ASSIGNMENT);
    }

    fn number(&mut self) {
        let val = (self.parser.previous.string())
            .parse::<f64>()
            .expect("number literal should be parseable as float");

        self.emit_constant(Value::Number(val));
    }

    fn emit_constant(&mut self, val: Value) {
        let c = self.make_constant(val);
        self.emit_byte(OpCode::Constant(c));
    }

    fn make_constant(&mut self, val: Value) -> usize {
        let constant = self.current_chunk().add_constant(val);
        if constant > (u8::MAX as usize) {
            self.error("Too many constants in one chunk.");
            return 0;
        }

        return constant;
    }

    fn grouping(&mut self) {
        self.expression();
        self.consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    }

    fn unary(&mut self) {
        let operator_type = self.parser.previous.token_type();

        // compile operand
        self.parse_precedence(Precedence::UNARY);

        // emit operator instruction
        match operator_type {
            TokenType::MINUS => self.emit_byte(OpCode::Negate),
            _ => unreachable!(),
        }
    }

    fn binary(&mut self) {
        let operatortype = self.parser.previous.token_type();

        let rule: ParseRule = self.rule(operatortype);

        let prec = rule.precedence();
        self.parse_precedence((prec as usize) + 1)

        match operatortype {
            TokenType::PLUS => {},
            TokenType::MINUS => {},
            TokenType::STAR => {},
            TokenType::SLASH => {},
            _ => unreachable!()
        }
    }

    fn parse_precedence(&mut self, precedence: Precedence) {}
}
