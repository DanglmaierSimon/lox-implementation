use crate::{chunk::Chunk, opcode::OpCode, value::Value};

pub fn disassemble_chunk(chunk: &Chunk, name: &str) {
    println!("== {} ==", name);

    for (idx, instruction) in chunk.code().iter().enumerate() {
        disassemble_instruction(idx, instruction, chunk);
    }
}

pub fn disassemble_instruction(idx: usize, instr: &OpCode, chunk: &Chunk) {
    print!("{:04} ", idx);

    if idx > 00 && chunk.lines()[idx] == chunk.lines()[idx - 1] {
        print!("   | ");
    } else {
        print!("{:4} ", chunk.lines()[idx])
    }

    match instr {
        OpCode::Return => simple_instruction("OP_RETURN"),
        OpCode::Constant(idx) => {
            return constant_instruction("OP_CONSTANT", chunk, *idx);
        }
    }
}

fn constant_instruction(name: &str, chunk: &Chunk, constant_idx: usize) {
    print!("{:<16} {:4} '", name, constant_idx);
    print_value(&chunk.constants()[constant_idx]);
    println!("'");
}

fn print_value(constant: &Value) {
    match constant {
        Value::Double(d) => print!("{}", *d),
    }
}

fn simple_instruction(name: &str) {
    println!("{}", name);
}
