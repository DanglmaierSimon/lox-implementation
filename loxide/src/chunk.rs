use std::ops::Index;

use crate::{
    opcode::OpCode,
    value::{Value, ValueArray},
};

pub struct Chunk {
    code: Vec<OpCode>,
    constants: ValueArray,
    lines: Vec<usize>,
}

impl Chunk {
    pub fn new() -> Self {
        return Self {
            code: Vec::new(),
            constants: ValueArray::new(),
            lines: Vec::new(),
        };
    }

    pub fn write(&mut self, byte: OpCode, line: usize) {
        self.code.push(byte);
        self.lines.push(line);
    }

    pub fn count(&self) -> usize {
        return self.code.len();
    }

    pub fn capacity(&self) -> usize {
        return self.code.capacity();
    }

    pub fn add_constant(&mut self, val: Value) -> usize {
        self.constants.write(val);
        return self.constants().count() - 1;
    }

    pub fn constants(&self) -> &ValueArray {
        return &self.constants;
    }

    pub fn code(&self) -> &Vec<OpCode> {
        return &self.code;
    }

    pub fn lines(&self) -> &[usize] {
        return self.lines.as_ref();
    }
}
