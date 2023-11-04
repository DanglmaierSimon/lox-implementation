use crate::{chunk::Chunk, object::Obj, opcode::OpCode, value::Value};

pub fn disassemble_chunk(chunk: &Chunk, name: &str) {
    println!("== {} ==", name);

    for (idx, instruction) in chunk.code().iter().enumerate() {
        disassemble_instruction(idx, *instruction, chunk);
    }
}

// TODO: Replace with an implementation of the Debug trait on the OpCode itself (possible?? see constant_instruction)
pub fn disassemble_instruction(idx: usize, instr: OpCode, chunk: &Chunk) {
    print!("{:04} ", idx);

    if idx > 0 && chunk.lines()[idx] == chunk.lines()[idx - 1] {
        print!("   | ");
    } else {
        print!("{:4} ", chunk.lines()[idx])
    }

    match instr {
        OpCode::Return => simple_instruction("OP_RETURN"),
        OpCode::Constant(idx) => {
            return constant_instruction("OP_CONSTANT", chunk, idx);
        }
        OpCode::Negate => simple_instruction("OP_NEGATE"),
        OpCode::Add => simple_instruction("OP_ADD"),
        OpCode::Subtract => simple_instruction("OP_SUB"),
        OpCode::Multiply => simple_instruction("OP_MULTIPY"),
        OpCode::Divide => simple_instruction("OP_DIVIDE"),
        OpCode::Nil => simple_instruction("OP_NIL"),
        OpCode::True => simple_instruction("OP_TRUE"),
        OpCode::False => simple_instruction("OP_FALSE"),
        OpCode::Not => simple_instruction("OP_NOT"),
        OpCode::Equal => simple_instruction("OP_EQUAL"),
        OpCode::Greater => simple_instruction("OP_GREATER"),
        OpCode::Less => simple_instruction("OP_LESS"),
        OpCode::Print => simple_instruction("OP_PRINT"),
        OpCode::Pop => simple_instruction("OP_POP"),
        OpCode::DefineGlobal(index) => constant_instruction("OP_DEFINE_GLOBAL", chunk, index),
        OpCode::GetGlobal(index) => constant_instruction("OP_GET_GLOBAL", chunk, index),
    }
}

fn constant_instruction(name: &str, chunk: &Chunk, constant_idx: usize) {
    print!("{:<16} {:4} '", name, constant_idx);
    print_value(&chunk.constants()[constant_idx]);
    println!("'");
}

pub fn print_value(constant: &Value) {
    match constant {
        Value::Number(d) => print!("{}", *d),
        Value::Nil => print!("nil"),
        Value::Bool(b) => print!("{}", b),
        Value::Obj(obj) => print_object(obj),
    }
}

fn simple_instruction(name: &str) {
    println!("{}", name);
}

fn print_object(obj: &Obj) {
    match obj {
        Obj::String(str) => print!("{}", str.str()),
    }
}
