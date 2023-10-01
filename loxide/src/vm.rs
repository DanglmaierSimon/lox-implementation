use std::rc::Rc;

use crate::{
    chunk::Chunk,
    compiler::Compiler,
    debug::{disassemble_instruction, print_value},
    opcode::OpCode,
    stack::Stack,
    value::Value,
};

#[derive(Debug)]
pub enum InterpretResult {
    Ok,
    CompileError,
    RuntimeError,
}

pub struct VM {
    chunk: Chunk,
    ip: usize,
    stack: Stack<Value, 256>,
}

impl VM {
    pub fn new(chunk: Chunk) -> Self {
        return Self {
            chunk,
            ip: 0,
            stack: Stack::new(),
        };
    }

    pub fn interpret(source: Rc<String>) -> InterpretResult {
        Compiler::compile(source);
        return InterpretResult::Ok;
    }

    pub fn run(&mut self) -> InterpretResult {
        loop {
            let instruction = &self.chunk.code()[self.ip];

            disassemble_instruction(self.ip, instruction, &self.chunk);

            print!("          ");
            {
                let &data = self.stack.data();
                for idx in 0..self.stack.count() {
                    print!("[");
                    print_value(&data[idx]);
                    print!("]");
                }
            }
            println!();

            self.ip += 1;

            match instruction {
                OpCode::Return => {
                    print_value(&self.pop());
                    println!();
                    return InterpretResult::Ok;
                }
                OpCode::Constant(idx) => {
                    let constant = &self.chunk.constants()[*idx];
                    self.push(*constant);
                    println!()
                }
                OpCode::Negate => {
                    let val = (-self.pop()).expect("runtime error todo!");
                    self.push(val)
                }
                OpCode::Add => {
                    let b = self.pop();
                    let a = self.pop();
                    let res = (a + b).expect("runtime error todo!");

                    self.push(res);
                }
                OpCode::Subtract => {
                    let b = self.pop();
                    let a = self.pop();
                    let res = (a - b).expect("runtime error todo!");

                    self.push(res);
                }
                OpCode::Multiply => {
                    let b = self.pop();
                    let a = self.pop();
                    let res = (a * b).expect("runtime error todo!");

                    self.push(res);
                }
                OpCode::Divide => {
                    let b = self.pop();
                    let a = self.pop();
                    let res = (a / b).expect("runtime error todo!");

                    self.push(res);
                }
            }
        }
    }

    fn pop(&mut self) -> Value {
        return self.stack.pop();
    }

    fn push(&mut self, val: Value) {
        self.stack.push(val);
    }
}
