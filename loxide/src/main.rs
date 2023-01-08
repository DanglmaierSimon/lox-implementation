#![allow(dead_code)] // TODO: Remove
use std::env;
use std::fs;
use std::io;

use parser::ParseError;
use parser::Parser;
use scanner::Scanner;
use token::{Token, TokenType};

use visitor::AstPrinter;

pub mod expr;
pub mod parser;
pub mod scanner;
pub mod token;
pub mod value;
pub mod visitor;

struct Runner {
    had_error: bool,
}

impl Runner {
    fn new() -> Runner {
        Runner { had_error: false }
    }

    fn run_file(&mut self, file_path: &String) {
        let contents = fs::read_to_string(file_path).expect("Could not read file.");

        println!("File content:\n{contents}");
        println!("==========================================");

        self.run(contents);

        if self.had_error {
            std::process::exit(exitcode::DATAERR)
        }
    }

    fn run_prompt(&mut self) {
        let stdin = io::stdin();

        loop {
            print!("> ");
            let mut line = String::new();
            stdin
                .read_line(&mut line)
                .expect("Could not read from stdin.");
            println!("input {} ", line);

            if line.is_empty() {
                break;
            }
            self.run(line);
            self.had_error = false;
        }
    }

    fn run(&mut self, source: String) {
        let tokens = Scanner::new(source).scan_tokens();
        let mut parser = Parser::new(tokens);
        let mut ast_printer = AstPrinter {};

        match parser.parse() {
            Ok(expr) => ast_printer.print_expr(expr),
            Err(err) => {
                println!("Error: {:?}", err);
                self.report_error(err)
            }
        }
    }

    fn error_with_token(&mut self, token: &Token, error_message: &str) {
        if token.token_type == TokenType::Eof {
            self.report(token.line, " at end", error_message)
        } else {
            self.report(
                token.line,
                &(" at '".to_owned() + &token.lexeme + "'"),
                error_message,
            ) // TODO: clean this up and learn rust strings properly
        }
    }

    fn error_at_line(&mut self, line: usize, error_message: &str) {
        self.report(line, "", error_message)
    }

    fn report_error(&mut self, err: ParseError) {
        match err {
            ParseError::TokenError { token, error_msg } => {
                self.error_with_token(&token, &error_msg)
            }
            ParseError::NumberConversionError {
                location,
                conversion_error,
            } => self.error_at_line(location.line, &conversion_error.to_string()),
        }
    }

    fn report(&mut self, line: usize, location: &str, error_message: &str) {
        self.had_error = true;
        println!("[line {}] Error {}: {}", line, location, error_message);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mut runner = Runner::new();

    if args.len() > 2 {
        println!("Usage: loxide [script]");
        std::process::exit(exitcode::USAGE);
    }

    if args.len() == 2 {
        runner.run_file(&args[1]);
    } else {
        runner.run_file(&"test.lox".to_owned());
        //runner.run_prompt();
    }
}
