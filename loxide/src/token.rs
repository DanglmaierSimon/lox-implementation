#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum TokenType {
    // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,
    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    ERROR,
    EOF,
}

pub struct Token {
    token_type: TokenType,
    string: String, // TOOD: find a more efficient representation for the substrings -> The full source code is available for the whole runtime of the program, maybe use a reference counter with a slice?
    line: usize,
}
impl Token {
    pub fn new(token_type: TokenType, substr: String, line: usize) -> Token {
        return Token {
            token_type: token_type,
            string: substr,
            line: line,
        };
    }

    pub fn token_type(&self) -> TokenType {
        return self.token_type;
    }

    pub fn line(&self) -> usize {
        return self.line;
    }

    pub fn string(&self) -> &str {
        return self.string.as_ref();
    }
}
