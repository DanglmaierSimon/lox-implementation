#![forbid(clippy::implicit_return)]
#![allow(clippy::needless_return)]

use std::env;

use chunk::Chunk;
use debug::disassemble_chunk;

mod chunk;
mod common;
mod debug;
mod opcode;
mod value;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() > 2 {
        println!("Usage: loxide [script]");
        std::process::exit(exitcode::USAGE);
    }

    let mut c = Chunk::new();
    let constant = c.add_constant(value::Value::Double(1.2));
    c.write(opcode::OpCode::Constant(constant), 123);

    c.write(opcode::OpCode::Return, 123);

    disassemble_chunk(&c, "test_chunk");

    return;
}
