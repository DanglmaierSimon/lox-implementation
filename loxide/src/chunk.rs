use std::ops::Index;

use crate::opcode::OpCode;

pub struct Chunk {
    code: Vec<OpCode>,
}

impl Index<usize> for Chunk {
    type Output = OpCode;

    fn index(&self, index: usize) -> &Self::Output {
        return &self.code[index];
    }
}

impl Chunk {
    pub fn new() -> Self {
        return Self { code: Vec::new() };
    }

    pub fn write(&mut self, byte: OpCode) {
        self.code.push(byte);
    }

    pub fn count(&self) -> usize {
        return self.code.len();
    }

    pub fn capacity(&self) -> usize {
        return self.code.capacity();
    }
}
