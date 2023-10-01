#![forbid(clippy::implicit_return)]
#![allow(clippy::needless_return)]

use std::env;

use chunk::Chunk;
use debug::disassemble_chunk;
use value::Value;
use vm::VM;

mod chunk;
mod common;
mod debug;
mod opcode;
mod stack;
mod value;
mod vm;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() > 2 {
        println!("Usage: loxide [script]");
        std::process::exit(exitcode::USAGE);
    }

    let mut c = Chunk::new();
    let constant = c.add_constant(Value::Number(1.2));
    c.write(opcode::OpCode::Constant(constant), 123);

    let constant2 = c.add_constant(Value::Number(3.4));
    c.write(opcode::OpCode::Constant(constant2), 123);

    c.write(opcode::OpCode::Add, 123);

    let constant3 = c.add_constant(Value::Number(5.6));
    c.write(opcode::OpCode::Constant(constant3), 123);

    c.write(opcode::OpCode::Divide, 123);

    c.write(opcode::OpCode::Negate, 123);
    c.write(opcode::OpCode::Return, 123);

    disassemble_chunk(&c, "test_chunk");

    match VM::interpret(c) {
        vm::InterpretResult::Ok => {}
        vm::InterpretResult::CompileError => {
            println!("compile error")
        }
        vm::InterpretResult::RuntimeError => {
            println!("runtime error")
        }
    }

    return;
}
