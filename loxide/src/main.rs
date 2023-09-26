#![forbid(clippy::implicit_return)]
#![allow(clippy::needless_return)]

use std::env;

use chunk::Chunk;
use debug::disassemble_chunk;

mod chunk;
mod common;
mod debug;
mod opcode;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() > 2 {
        println!("Usage: loxide [script]");
        std::process::exit(exitcode::USAGE);
    }

    let mut c = Chunk::new();

    c.write(opcode::OpCode::Return);

    disassemble_chunk(&c, "test_chunk");

    return;
}
