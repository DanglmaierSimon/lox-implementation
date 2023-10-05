use std::rc::Rc;

use crate::token::{Token, TokenType};

pub struct Scanner {
    source: Rc<String>,
    line: usize,
    start: usize,
    current: usize,
}

impl Scanner {
    pub fn new(source: Rc<String>) -> Self {
        return Scanner {
            source,
            line: 1,
            start: 0,
            current: 0,
        };
    }

    pub fn scan_token(&mut self) -> Token {
        self.skip_whitespace();

        self.start = self.current;

        if self.is_at_end() {
            return self.make_token(TokenType::EOF);
        }

        let c = self.advance();

        if c.is_ascii_digit() {
            return self.number();
        }

        if is_alphanumeric(c) {
            return self.identifier();
        }

        match c {
            '(' => {
                return self.make_token(TokenType::LEFT_PAREN);
            }
            ')' => {
                return self.make_token(TokenType::RIGHT_PAREN);
            }
            '{' => {
                return self.make_token(TokenType::LEFT_BRACE);
            }
            '}' => {
                return self.make_token(TokenType::RIGHT_BRACE);
            }
            ';' => {
                return self.make_token(TokenType::SEMICOLON);
            }
            ',' => {
                return self.make_token(TokenType::COMMA);
            }
            '.' => {
                return self.make_token(TokenType::DOT);
            }
            '-' => {
                return self.make_token(TokenType::MINUS);
            }
            '+' => {
                return self.make_token(TokenType::PLUS);
            }
            '/' => {
                return self.make_token(TokenType::SLASH);
            }
            '*' => {
                return self.make_token(TokenType::STAR);
            }
            '!' => {
                let t = if self.match_char('=') {
                    TokenType::BANG_EQUAL
                } else {
                    TokenType::BANG
                };
                return self.make_token(t);
            }
            '=' => {
                let t = if self.match_char('=') {
                    TokenType::EQUAL_EQUAL
                } else {
                    TokenType::EQUAL
                };
                return self.make_token(t);
            }
            '<' => {
                let t = if self.match_char('=') {
                    TokenType::LESS_EQUAL
                } else {
                    TokenType::LESS
                };
                return self.make_token(t);
            }
            '>' => {
                let t = if self.match_char('=') {
                    TokenType::GREATER_EQUAL
                } else {
                    TokenType::GREATER
                };
                return self.make_token(t);
            }
            '"' => {
                return self.string();
            }
            _ => {}
        }
        return self.error_token("Unexpected character.");
    }

    fn is_at_end(&self) -> bool {
        return self.current == self.source.len();
    }

    fn error_token(&self, error: &str) -> Token {
        return Token::new(TokenType::ERROR, error.to_string(), self.line);
    }

    fn make_token(&self, ttype: TokenType) -> Token {
        let substr = self
            .source
            .chars() // TODO: make this handle unicode properly
            .skip(self.start)
            .take(self.current - self.start)
            .collect();

        return Token::new(ttype, substr, self.line);
    }

    fn advance(&mut self) -> char {
        self.current += 1;
        return self.source.chars().nth(self.current - 1).unwrap(); // TODO: are iterators fast? is there a way to index strings directly?
    }

    fn match_char(&mut self, expected: char) -> bool {
        if self.is_at_end() {
            return false;
        }

        let nth_char = self.source.chars().nth(self.current);

        match nth_char {
            None => return false,
            Some(c) => {
                if c != expected {
                    return false;
                }
            }
        }

        self.current += 1;
        return true;
    }

    fn skip_whitespace(&mut self) {
        loop {
            let c = self.peek();

            match c {
                ' ' | '\r' | '\t' => {
                    self.advance();
                }
                '\n' => {
                    self.line += 1;
                    self.advance();
                }
                '/' => {
                    if self.peek_next() == '/' {
                        while self.peek() != '\n' && !self.is_at_end() {
                            self.advance();
                        }
                    } else {
                        return;
                    };
                }
                _ => return,
            };
        }
    }

    fn peek(&self) -> char {
        return self.source.chars().nth(self.current).unwrap();
    }

    fn peek_next(&self) -> char {
        match self.is_at_end() {
            true => return '\0',
            false => {
                return self
                    .source
                    .chars()
                    .nth(self.current + 1)
                    .expect("Source terminated early!")
            }
        }
    }

    fn string(&mut self) -> Token {
        while self.peek() != '"' && !self.is_at_end() {
            if self.peek() == '\n' {
                self.line += 1;
            }
            self.advance();
        }

        if self.is_at_end() {
            return self.error_token("Unterminated string.");
        }

        // closing quote
        self.advance();
        return self.make_token(TokenType::STRING);
    }

    fn number(&mut self) -> Token {
        while self.peek().is_ascii_digit() {
            self.advance();
        }

        // fractional part
        if self.peek() == '.' && self.peek_next().is_ascii_digit() {
            self.advance(); // consume the '.'

            while self.peek().is_ascii_digit() {
                self.advance();
            }
        }

        return self.make_token(TokenType::NUMBER);
    }

    fn identifier(&mut self) -> Token {
        while is_alphanumeric(self.peek()) || self.peek().is_ascii_digit() {
            self.advance();
        }

        return self.make_token(self.identifier_type());
    }

    fn identifier_type(&self) -> TokenType {
        match self.source.chars().next().unwrap() {
            'a' => {
                return self.check_keyword(1, 2, "nd", TokenType::AND);
            }
            'c' => {
                return self.check_keyword(1, 4, "lass", TokenType::CLASS);
            }
            'e' => {
                return self.check_keyword(1, 3, "lse", TokenType::ELSE);
            }
            'i' => {
                return self.check_keyword(1, 1, "f", TokenType::IF);
            }
            'n' => {
                return self.check_keyword(1, 2, "il", TokenType::NIL);
            }
            'o' => {
                return self.check_keyword(1, 1, "r", TokenType::OR);
            }
            'p' => {
                return self.check_keyword(1, 4, "rint", TokenType::PRINT);
            }
            'r' => {
                return self.check_keyword(1, 5, "eturn", TokenType::RETURN);
            }
            's' => {
                return self.check_keyword(1, 4, "uper", TokenType::SUPER);
            }
            'v' => {
                return self.check_keyword(1, 2, "ar", TokenType::VAR);
            }
            'w' => {
                return self.check_keyword(1, 4, "hile", TokenType::WHILE);
            }
            'f' => {
                if self.current - self.start > 1 {
                    match self.source.chars().nth(1).unwrap() {
                        'a' => {
                            return self.check_keyword(2, 3, "lse", TokenType::FALSE);
                        }
                        'o' => {
                            return self.check_keyword(2, 1, "r", TokenType::FOR);
                        }
                        'u' => {
                            return self.check_keyword(2, 1, "n", TokenType::FUN);
                        }
                        _ => {}
                    }
                }
            }
            't' => {
                if self.current - self.start > 1 {
                    match self.source.chars().nth(1).unwrap() {
                        'h' => {
                            return self.check_keyword(2, 2, "is", TokenType::THIS);
                        }
                        'r' => {
                            return self.check_keyword(2, 2, "ue", TokenType::TRUE);
                        }
                        _ => {}
                    }
                }
            }
            _ => {}
        }

        return TokenType::IDENTIFIER;
    }

    fn check_keyword(&self, start: usize, len: usize, rest: &str, ttype: TokenType) -> TokenType {
        if self.current - self.start == start + len {
            let source_substr = &self.source[self.start + start..self.start + start + len];
            if rest == source_substr {
                return ttype;
            }
        }

        return TokenType::IDENTIFIER;
    }
}

const fn is_alphanumeric(c: char) -> bool {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
