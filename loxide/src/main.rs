#![forbid(clippy::implicit_return)]
#![allow(clippy::needless_return)]
#![allow(dead_code)]
#![allow(non_camel_case_types)]
#![allow(clippy::upper_case_acronyms)]

use std::{env, io, rc::Rc};

use vm::VM;

mod chunk;
mod common;
mod compiler;
mod debug;
mod opcode;
mod scanner;
mod stack;
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

    // let mut c = Chunk::new();
    // let constant = c.add_constant(Value::Number(1.2));
    // c.write(opcode::OpCode::Constant(constant), 123);

    // let constant2 = c.add_constant(Value::Number(3.4));
    // c.write(opcode::OpCode::Constant(constant2), 123);

    // c.write(opcode::OpCode::Add, 123);

    // let constant3 = c.add_constant(Value::Number(5.6));
    // c.write(opcode::OpCode::Constant(constant3), 123);

    // c.write(opcode::OpCode::Divide, 123);

    // c.write(opcode::OpCode::Negate, 123);
    // c.write(opcode::OpCode::Return, 123);

    // disassemble_chunk(&c, "test_chunk");

    // match VM::interpret(c) {
    //     vm::InterpretResult::Ok => {}
    //     vm::InterpretResult::CompileError => {
    //         println!("compile error")
    //     }
    //     vm::InterpretResult::RuntimeError => {
    //         println!("runtime error")
    //     }
    // }

    return;
}

fn repl() {
    loop {
        let mut line = String::new();

        print!("> ");

        match io::stdin().read_line(&mut line) {
            Ok(_) => {
                let line = Rc::new(line);

                let mut vm = VM::new();

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
