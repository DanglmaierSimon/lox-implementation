use std::rc::Rc;

use crate::{scanner::Scanner, token::TokenType};

pub struct Compiler {}

impl Compiler {
    pub fn compile(source: Rc<String>) {
        let mut scanner = Scanner::new(source);

        let mut line = 0;

        loop {
            let token = scanner.scan_token();
            if token.line() != line {
                print!("{:4} ", token.line());
                line = token.line();
            } else {
                print!("   | ");
            }

            println!("{:?} '{}'", token.token_type(), token.string());

            if token.token_type() == TokenType::EOF {
                break;
            }
        }
    }
}
