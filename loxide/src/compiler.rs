use crate::{
    chunk::Chunk,
    opcode::OpCode,
    scanner::Scanner,
    token::{Token, TokenType},
    value::Value,
};

#[derive(PartialEq, PartialOrd, Debug, Copy, Clone)]
enum Precedence {
    NONE = 0,
    ASSIGNMENT, // =
    OR,         // or
    AND,        // and
    EQUALITY,   // == !=
    COMPARISON, // < > <= >=
    TERM,       // + -
    FACTOR,     // * /
    UNARY,      // ! -
    CALL,       // . ()
    PRIMARY,
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

    fn parse_precedence(&mut self, precedence: Precedence) {}
}
