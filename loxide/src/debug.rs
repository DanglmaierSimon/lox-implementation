use crate::{chunk::Chunk, opcode::OpCode};

pub fn disassemble_chunk(chunk: &Chunk, name: &str) {
    println!("== {} ==", name);
    let mut offset = 0;

    while offset < chunk.count() {
        offset = disassemble_instruction(chunk, offset);
    }
}

pub fn disassemble_instruction(chunk: &Chunk, offset: usize) -> usize {
    print!("{:04} ", offset);

    let instr = &chunk[offset];
    match instr {
        OpCode::Return => {
            return simple_instruction("OP_RETURN", offset);
        }
        _ => {
            println!("unknown opcode: {:?}", instr);
            return offset + 1;
        }
    }
}

fn simple_instruction(name: &str, offset: usize) -> usize {
    println!("{}", name);
    return offset + 1;
}
