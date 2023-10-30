use crate::{
    opcode::OpCode,
    value::{Value, ValueArray},
};

// TODO: replace the dissassemble_Instruction functions with an impleemntation of the rust Debug trait
pub struct Chunk {
    // TODO: Maybe change modelling of bytecode to be more in line with what the book implementation does and hide it all within this class
    // use a vector of u8 and do runtime checks -> measure performance difference
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
