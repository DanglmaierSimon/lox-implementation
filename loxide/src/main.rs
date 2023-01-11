#![allow(dead_code)] // TODO: Remove
use std::env;
use std::fs;
use std::io;

use interpreter::Interpreter;
use interpreter::RuntimeError;
use parser::ParseError;
use parser::Parser;
use scanner::Scanner;
use token::{Token, TokenType};

pub mod environment;
pub mod expr;
pub mod interpreter;
pub mod parser;
pub mod scanner;
pub mod token;
pub mod value;

struct Runner<'a> {
    had_error: bool,
    had_runtime_error: bool,
    interpreter: Interpreter<'a>,
}

impl<'a> Runner<'a> {
    fn new() -> Runner<'a> {
        Runner {
            had_error: false,
            had_runtime_error: false,
            interpreter: Interpreter::new(),
        }
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
        //let mut ast_printer = AstPrinter {};

        match parser.parse() {
            Ok(statements) => match self.interpreter.interpret(statements) {
                Some(err) => {
                    println!("Error: {:?}", err);
                    self.runtime_error(err)
                }
                None => {}
            },
            Err(err) => self.report_error(err),
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

    fn runtime_error(&mut self, err: RuntimeError) {
        println!("{}\n[line {}]", err.msg, err.token.line);
        self.had_runtime_error = true;
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
