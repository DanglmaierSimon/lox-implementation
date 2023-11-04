#![forbid(clippy::implicit_return)]
#![allow(clippy::needless_return)]
#![allow(dead_code)]
#![allow(non_camel_case_types)]
#![allow(clippy::upper_case_acronyms)]

use std::{
    env,
    io::{self, Write},
    rc::Rc,
};

use vm::VM;

mod chunk;
mod common;
mod compiler;
mod debug;
mod gc;
mod object;
mod opcode;
mod scanner;
mod stack;
mod table;
mod token;
mod value;
mod vm;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() == 1 {
        repl();
    } else if args.len() == 2 {
        run_file(&args[1]);
    } else {
        eprintln!("Usage: loxide [script]");
        std::process::exit(exitcode::USAGE);
    }

    return;
}

fn repl() {
    let mut vm = VM::new();

    loop {
        let mut line = String::new();

        print!("> ");

        _ = std::io::stdout().flush();

        match io::stdin().read_line(&mut line) {
            Ok(_) => {
                let line = Rc::new(line);

                vm.interpret(line);
            }
            Err(err) => {
                eprintln!("Error reading line from stdin: {}", err);
                break;
            }
        }
    }
}

fn run_file(filepath: &String) {
    let mut vm = VM::new();
    let source = Rc::new(read_file(filepath));
    let res = vm.interpret(source);
    match res {
        vm::InterpretResult::Ok => {}
        vm::InterpretResult::CompileError => {
            println!("compile error");
            std::process::exit(exitcode::DATAERR);
        }
        vm::InterpretResult::RuntimeError => {
            println!("runtime error");
            std::process::exit(exitcode::SOFTWARE);
        }
    }
}

fn read_file(filepath: &String) -> String {
    match std::fs::read_to_string(filepath) {
        Ok(str) => return str,
        Err(err) => {
            eprintln!("could not open file {} for reading: {}", filepath, err);
            std::process::exit(exitcode::IOERR);
        }
    }
}
