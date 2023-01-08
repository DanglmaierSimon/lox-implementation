use std::collections::HashMap;

use crate::token::Token;
use crate::token::TokenType;

pub struct Scanner {
    source: String,
    start: usize,
    current: usize,
    line: usize,
}

impl Scanner {
    pub fn new(source: String) -> Scanner {
        return Scanner {
            source,
            start: 0,
            current: 0,
            line: 1,
        };
    }

    pub fn scan_tokens(&mut self) -> Vec<Token> {
        let mut retval = Vec::new();

        loop {
            let t = self.scan_token();

            if t.token_type == TokenType::Error || t.token_type == TokenType::Eof {
                retval.push(t);
                return retval;
            }
            retval.push(t);
        }
    }

    pub fn scan_token(&mut self) -> Token {
        if self.is_at_end() {
            return self.make_token(TokenType::Eof);
        }

        self.skip_whitespace();

        self.start = self.current;

        let c = self.advance();

        if char::is_digit(c, 10) {
            return self.number();
        }
        if is_alpha(c) {
            return self.identifier();
        }

        match c {
            '(' => return self.make_token(TokenType::LeftParen),
            ')' => return self.make_token(TokenType::RightParen),
            '{' => return self.make_token(TokenType::LeftBrace),
            '}' => return self.make_token(TokenType::RightBrace),
            ';' => return self.make_token(TokenType::Semicolon),
            ',' => return self.make_token(TokenType::Comma),
            '.' => return self.make_token(TokenType::Dot),
            '-' => return self.make_token(TokenType::Minus),
            '+' => return self.make_token(TokenType::Plus),
            '/' => return self.make_token(TokenType::Slash),
            '*' => return self.make_token(TokenType::Star),
            '!' => {
                let t = match self.match_char('=') {
                    true => TokenType::BangEqual,
                    false => TokenType::Bang,
                };
                return self.make_token(t);
            }
            '=' => {
                let t = match self.match_char('=') {
                    true => TokenType::EqualEqual,
                    false => TokenType::Equal,
                };

                return self.make_token(t);
            }
            '<' => {
                let t = match self.match_char('=') {
                    true => TokenType::LessEqual,
                    false => TokenType::Less,
                };
                return self.make_token(t);
            }
            '>' => {
                let t = match self.match_char('=') {
                    true => TokenType::GreaterEqual,
                    false => TokenType::Greater,
                };
                return self.make_token(t);
            }
            '"' => return self.string(),

            _ => (),
        }

        return self.error_token("Unexpected character!");
    }

    fn is_at_end(&self) -> bool {
        return self.source.len() <= self.current;
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
        if self.is_at_end() {
            return '\0';
        }

        return self
            .source
            .chars()
            .nth(self.current)
            .expect("Source string shorter than expected!");
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

    // fn scan_token(&mut self) {
    //     let c = self.advance();

    //     match c {
    //         '(' => self.add_token(TokenType::LEFT_PAREN),
    //         ')' => self.add_token(TokenType::RIGHT_PAREN),
    //         '{' => self.add_token(TokenType::LEFT_BRACE),
    //         '}' => self.add_token(TokenType::RIGHT_BRACE),
    //         ',' => self.add_token(TokenType::COMMA),
    //         '.' => self.add_token(TokenType::DOT),
    //         '-' => self.add_token(TokenType::MINUS),
    //         '+' => self.add_token(TokenType::PLUS),
    //         ';' => self.add_token(TokenType::SEMICOLON),
    //         '*' => self.add_token(TokenType::STAR),

    //         _ => {
    //             eprint!("unexpected character at line {}", self.line);
    //             self.had_error = true
    //         }
    //     }
    // }

    fn advance(&mut self) -> char {
        let c = self
            .source
            .chars()
            .nth(self.current)
            .expect("Index out of range!");
        self.current += 1;
        return c;
    }

    // fn add_token(&mut self, token_type: TokenType) {
    //     self.add_token_with_lit(token_type);
    // }

    // fn add_token_with_lit(&mut self, token_type: TokenType /*literal : Object */) {
    //     let text: String = self
    //         .source
    //         .chars()
    //         .skip(self.current)
    //         .take(self.current - self.start)
    //         .collect();
    //     self.tokens.push(Token::new(token_type, text, self.line))
    // }

    fn make_token(&self, token_type: TokenType) -> Token {
        let substr = self
            .source
            .chars()
            .skip(self.start)
            .take(self.current - self.start)
            .collect();

        return Token::new(token_type, substr, self.line);
    }

    fn error_token(&self, error_msg: &str) -> Token {
        return Token::new(TokenType::Error, error_msg.to_string(), self.line);
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
        return self.make_token(TokenType::String);
    }

    fn number(&mut self) -> Token {
        while char::is_digit(self.peek(), 10) {
            self.advance();
        }

        if self.peek() == '.' && char::is_digit(self.peek_next(), 10) {
            self.advance();

            while char::is_digit(self.peek(), 10) {
                self.advance();
            }
        }

        return self.make_token(TokenType::Number);
    }

    fn identifier(&mut self) -> Token {
        while is_alpha(self.peek()) || char::is_digit(self.peek(), 10) {
            self.advance();
        }

        return self.make_token(self.identifier_type());
    }

    fn identifier_type(&self) -> TokenType {
        let keywords = HashMap::<&str, TokenType>::from([
            ("and", TokenType::And),
            ("class", TokenType::Class),
            ("else", TokenType::Else),
            ("if", TokenType::If),
            ("nil", TokenType::Nil),
            ("or", TokenType::Or),
            ("print", TokenType::Print),
            ("return", TokenType::Return),
            ("super", TokenType::Super),
            ("var", TokenType::Var),
            ("while", TokenType::While),
            ("false", TokenType::False),
            ("for", TokenType::For),
            ("fun", TokenType::Fun),
            ("this", TokenType::This),
            ("true", TokenType::True),
        ]);

        let substr: String = self
            .source
            .chars()
            .skip(self.start)
            .take(self.current - self.start)
            .collect();

        return keywords
            .get(&substr[..])
            .copied()
            .unwrap_or(TokenType::Identifier);
    }
}

fn is_alpha(c: char) -> bool {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
