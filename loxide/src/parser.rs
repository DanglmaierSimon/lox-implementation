use std::num::ParseFloatError;

use crate::{
    expr::{self, Expr},
    token::{Token, TokenType},
    value::LoxValue,
};

#[derive(Debug)]
pub struct SourceLocation {
    pub lexeme: String,
    pub line: usize,
}

#[derive(Debug)]
pub enum ParseError {
    TokenError {
        token: Token,
        error_msg: String, // TODO: use static string here!
    },
    NumberConversionError {
        location: SourceLocation,
        conversion_error: ParseFloatError,
    },
}

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Parser {
        return Parser { tokens, current: 0 };
    }

    fn assignment(&mut self) -> Result<Expr, ParseError> {
        let expr = self.or()?;

        if self.match_token(&[TokenType::Equal]) {
            let equals = self.previous();

            // assignment is right-associative -> recursively call assignment to parse the right-hand side
            let value = self.assignment()?;

            if expr.is_variable() {
                let name = expr.as_variable().expect("cast to variable must succeed!");
                return Ok(Expr::Assignment {
                    name: name,
                    value: Box::new(value),
                });
            }

            return Err(self.error(equals, "Invalid assignment target."));
        }

        return Ok(expr);
    }

    fn expression(&mut self) -> Result<Expr, ParseError> {
        return self.assignment();
    }

    fn equality(&mut self) -> Result<Expr, ParseError> {
        let mut expr = self.comparison()?;

        while self.match_token(&[TokenType::BangEqual, TokenType::EqualEqual]) {
            let operator = self.previous();
            let right = self.comparison()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                operator: operator.clone(),
                right: Box::new(right),
            }
        }

        return Ok(expr);
    }

    fn match_token(&mut self, tokens: &[TokenType]) -> bool {
        for t in tokens {
            if self.check(*t) {
                self.advance();
                return true;
            }
        }

        return false;
    }

    fn check(&self, t: TokenType) -> bool {
        if self.is_at_end() {
            return false;
        }

        return self.peek().token_type == t;
    }

    fn advance(&mut self) -> Token {
        if !self.is_at_end() {
            self.current += 1;
        }

        return self.previous();
    }

    fn is_at_end(&self) -> bool {
        return self.peek().token_type == TokenType::Eof;
    }

    fn peek(&self) -> Token {
        match self.tokens.get(self.current) {
            Some(token) => return token.clone(),
            None => {
                println!("Reached the end of the token stream! This should never happen. Returning EOF token!");
                return Token::new(TokenType::Eof, "".to_owned(), 0);
            }
        }
    }

    fn previous(&mut self) -> Token {
        match self.tokens.get(self.current - 1) {
            Some(token) => return token.clone(),
            None => {
                println!("Could not retrieve previous token! This should never happen. Returning EOF token!");
                return Token::new(TokenType::Eof, "".to_owned(), 0);
            }
        }
    }

    fn comparison(&mut self) -> Result<Expr, ParseError> {
        let mut expr = self.term()?;

        while self.match_token(&[
            TokenType::Greater,
            TokenType::GreaterEqual,
            TokenType::Less,
            TokenType::LessEqual,
        ]) {
            let operator = self.previous();
            let right = self.term()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                operator: operator.clone(),
                right: Box::new(right),
            }
        }

        return Ok(expr);
    }

    fn term(&mut self) -> Result<Expr, ParseError> {
        let mut expr = self.factor()?;

        while self.match_token(&[TokenType::Minus, TokenType::Plus]) {
            let operator = self.previous();
            let right = self.factor()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                operator: operator.clone(),
                right: Box::new(right),
            }
        }

        return Ok(expr);
    }

    fn factor(&mut self) -> Result<Expr, ParseError> {
        let mut expr = self.unary()?;

        while self.match_token(&[TokenType::Star, TokenType::Slash]) {
            let operator = self.previous();
            let right = self.unary()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                operator: operator.clone(),
                right: Box::new(right),
            }
        }

        return Ok(expr);
    }

    fn unary(&mut self) -> Result<Expr, ParseError> {
        if self.match_token(&[TokenType::Bang, TokenType::Minus]) {
            let operator = self.previous();
            let right = self.unary()?;
            return Ok(Expr::Unary {
                operator: operator.clone(),
                right: Box::new(right),
            });
        }

        return self.primary();
    }

    fn primary(&mut self) -> Result<Expr, ParseError> {
        if self.match_token(&[TokenType::False]) {
            return Ok(Expr::Literal {
                value: LoxValue::Bool(false),
            });
        }

        if self.match_token(&[TokenType::True]) {
            return Ok(Expr::Literal {
                value: LoxValue::Bool(true),
            });
        }

        if self.match_token(&[TokenType::Nil]) {
            return Ok(Expr::Literal {
                value: LoxValue::Nil(),
            });
        }

        if self.match_token(&[TokenType::Number]) {
            match self.previous().lexeme.parse::<f64>() {
                Ok(value) => {
                    return Ok(Expr::Literal {
                        value: LoxValue::Number(value),
                    })
                }
                Err(parse_error) => {
                    return Err(ParseError::NumberConversionError {
                        conversion_error: parse_error,
                        location: SourceLocation {
                            lexeme: self.previous().lexeme,
                            line: self.previous().line,
                        },
                    })
                }
            }
        }

        if self.match_token(&[TokenType::String]) {
            return Ok(Expr::Literal {
                value: LoxValue::String(self.previous().lexeme),
            });
        }

        if self.match_token(&[TokenType::Identifier]) {
            return Ok(expr::Expr::Variable {
                name: self.previous(),
            });
        }

        if self.match_token(&[TokenType::LeftParen]) {
            let expr = self.expression()?;
            self.consume(TokenType::RightParen, "Expect '(' after expression.")?;
            return Ok(Expr::Grouping {
                expr: Box::new(expr),
            });
        }

        return Err(self.error(self.peek(), "Expect expression."));
    }

    fn consume(&mut self, token_type: TokenType, error_msg: &str) -> Result<Token, ParseError> {
        if self.check(token_type) {
            return Ok(self.advance());
        }

        return Err(self.error(self.peek(), error_msg));
    }

    fn error(&mut self, token: Token, error_msg: &str) -> ParseError {
        return ParseError::TokenError {
            token,
            error_msg: error_msg.to_owned(),
        };
    }

    pub fn parse(&mut self) -> Result<Vec<expr::Stmt>, ParseError> {
        let mut retval = Vec::new();

        while !self.is_at_end() {
            retval.push(self.declaration()?);
        }

        return Ok(retval);
    }

    fn statement(&mut self) -> Result<expr::Stmt, ParseError> {
        if self.match_token(&[TokenType::For]) {
            return self.for_statement();
        }

        if self.match_token(&[TokenType::If]) {
            return self.if_statement();
        }

        if self.match_token(&[TokenType::Print]) {
            return self.print_statement();
        }

        if self.match_token(&[TokenType::While]) {
            return self.while_statement();
        }

        if self.match_token(&[TokenType::LeftBrace]) {
            return Ok(expr::Stmt::Block {
                statements: self.block()?,
            });
        }

        return self.expression_statement();
    }

    fn print_statement(&mut self) -> Result<expr::Stmt, ParseError> {
        let expr = self.expression()?;
        self.consume(TokenType::Semicolon, "Expect ';' after value.")?;
        return Ok(expr::Stmt::Print { expr });
    }

    fn expression_statement(&mut self) -> Result<expr::Stmt, ParseError> {
        let expr = self.expression()?;
        self.consume(TokenType::Semicolon, "Expect ';' after expression.")?;
        return Ok(expr::Stmt::Expression { expr });
    }

    fn declaration(&mut self) -> Result<expr::Stmt, ParseError> {
        let retval: Result<expr::Stmt, ParseError>;

        if self.match_token(&[TokenType::Var]) {
            retval = self.var_declaration();
        } else {
            retval = self.statement();
        }

        match retval {
            Ok(_) => {
                return retval;
            }
            Err(_) => {
                self.synchronize();
                return retval;
            }
        }
    }

    fn var_declaration(&mut self) -> Result<expr::Stmt, ParseError> {
        let name = self.consume(TokenType::Identifier, "Expect variable name.")?;

        let mut initializer: Option<Expr> = None;
        if self.match_token(&[TokenType::Equal]) {
            match self.expression() {
                Ok(expr) => initializer = Some(expr),
                Err(err) => return Err(err),
            }
        }

        self.consume(
            TokenType::Semicolon,
            "Expect ';' after variable declaration.",
        )?;
        return Ok(expr::Stmt::Variable { name, initializer });
    }

    fn synchronize(&mut self) {
        _ = self.advance();

        while !self.is_at_end() {
            if self.previous().token_type == TokenType::Semicolon {
                return;
            }

            match self.peek().token_type {
                TokenType::Class
                | TokenType::Fun
                | TokenType::Var
                | TokenType::For
                | TokenType::If
                | TokenType::While
                | TokenType::Print
                | TokenType::Return => return,
                _ => {
                    self.advance();
                }
            }
        }
    }

    fn block(&mut self) -> Result<Vec<expr::Stmt>, ParseError> {
        let mut statements: Vec<expr::Stmt> = Vec::new();

        while !self.check(TokenType::RightBrace) && !self.is_at_end() {
            statements.push(self.declaration()?);
        }

        _ = self.consume(TokenType::RightBrace, "Expect '}' after block.")?;
        Ok(statements)
    }

    fn if_statement(&mut self) -> Result<expr::Stmt, ParseError> {
        self.consume(TokenType::LeftParen, "Expect '(' after 'if'.")?;
        let condition = self.expression()?;
        self.consume(TokenType::RightParen, "Expect ')' after if condition.")?;

        let then_branch = self.statement()?;
        let else_branch = if self.match_token(&[TokenType::Else]) {
            Some(Box::new(self.statement()?))
        } else {
            None
        };

        return Ok(expr::Stmt::If {
            condition,
            then_branch: Box::new(then_branch),
            else_branch,
        });
    }

    fn or(&mut self) -> Result<Expr, ParseError> {
        let mut expr = self.and()?;

        while self.match_token(&[TokenType::Or]) {
            let operator = self.previous();
            let rhs = self.and()?;
            expr = Expr::Logical {
                left: Box::new(expr),
                operator,
                right: Box::new(rhs),
            }
        }

        return Ok(expr);
    }

    fn and(&mut self) -> Result<Expr, ParseError> {
        let mut expr = self.equality()?;

        while self.match_token(&[TokenType::And]) {
            let operator = self.previous();
            let rhs = self.equality()?;
            expr = Expr::Logical {
                left: Box::new(expr),
                operator,
                right: Box::new(rhs),
            }
        }

        return Ok(expr);
    }

    fn while_statement(&mut self) -> Result<expr::Stmt, ParseError> {
        self.consume(TokenType::LeftParen, "Expect '(' after 'while'.")?;
        let condition = self.expression()?;
        self.consume(TokenType::RightParen, "Expect ')' after condition.")?;
        let body = self.statement()?;

        Ok(expr::Stmt::While {
            condition,
            body: Box::new(body),
        })
    }

    fn for_statement(&mut self) -> Result<expr::Stmt, ParseError> {
        self.consume(TokenType::LeftParen, "Expect '(' after 'for'.")?;

        let initializer = if self.match_token(&[TokenType::Semicolon]) {
            None
        } else if self.match_token(&[TokenType::Var]) {
            Some(self.var_declaration()?)
        } else {
            Some(self.expression_statement()?)
        };

        let mut condition = if !self.check(TokenType::Semicolon) {
            Some(self.expression()?)
        } else {
            None
        };

        self.consume(TokenType::Semicolon, "Expect ';' after loop condition.")?;

        let increment = if !self.check(TokenType::RightParen) {
            Some(self.expression()?)
        } else {
            None
        };

        self.consume(TokenType::RightParen, "Expect ')' after for clauses.")?;
        let mut body = self.statement()?;

        if let Some(inc) = increment {
            body = expr::Stmt::Block {
                statements: Vec::from([body, expr::Stmt::Expression { expr: inc }]),
            }
        }

        if condition.is_none() {
            condition = Some(expr::Expr::Literal {
                value: LoxValue::Bool(true),
            })
        }

        body = expr::Stmt::While {
            condition: condition.unwrap(), // TODO: rewrite this desugaring logic to not need this unwrap!
            body: Box::new(body),
        };

        if let Some(init) = initializer {
            body = expr::Stmt::Block {
                statements: Vec::from([init, body]),
            }
        }

        Ok(body)
    }
}
