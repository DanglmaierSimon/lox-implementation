use std::rc::Rc;

use crate::{
    chunk::Chunk,
    compiler::{compile, Compiler},
    debug::{disassemble_instruction, print_value},
    opcode::OpCode,
    scanner::Scanner,
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
    pub fn new() -> Self {
        return Self {
            chunk: Chunk::new(),
            ip: 0,
            stack: Stack::new(),
        };
    }

    pub fn interpret(&mut self, source: Rc<String>) -> InterpretResult {
        let scanner = Scanner::new(Rc::clone(&source));
        let compiler = Compiler::new(scanner);

        let compile_result = compile(compiler);

        if !(compile_result.0) {
            return InterpretResult::CompileError;
        }

        self.chunk = compile_result.1;
        self.ip = 0;

        return self.run();
    }

    pub fn run(&mut self) -> InterpretResult {
        loop {
            let instruction = &self.chunk.code()[self.ip];

            disassemble_instruction(self.ip, instruction, &self.chunk);

            print!("          ");
            {
                for val in self.stack.data().iter().take(self.stack.count()) {
                    print!("[");
                    print_value(val);
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
                OpCode::Negate => match self.pop().as_number() {
                    Some(n) => self.push(Value::Number(-n)),
                    None => {
                        self.runtime_error("Operand must be a number.");
                        return InterpretResult::RuntimeError;
                    }
                },
                OpCode::Add => {
                    if !self.peek(0).is_number() || !self.peek(1).is_number() {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }

                    let b = *self.pop().as_number().expect("Operands must be numbers.");
                    let a = *self.pop().as_number().expect("Operands must be numbers.");

                    self.push(Value::Number(a + b));
                }
                OpCode::Subtract => {
                    if !self.peek(0).is_number() || !self.peek(1).is_number() {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }

                    let b = *self.pop().as_number().expect("Operands must be numbers.");
                    let a = *self.pop().as_number().expect("Operands must be numbers.");

                    self.push(Value::Number(a - b));
                }
                OpCode::Multiply => {
                    if !self.peek(0).is_number() || !self.peek(1).is_number() {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }

                    let b = *self.pop().as_number().expect("Operands must be numbers.");
                    let a = *self.pop().as_number().expect("Operands must be numbers.");

                    self.push(Value::Number(a * b));
                }
                OpCode::Divide => {
                    if !self.peek(0).is_number() || !self.peek(1).is_number() {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }

                    let b = *self.pop().as_number().expect("Operands must be numbers.");
                    let a = *self.pop().as_number().expect("Operands must be numbers.");

                    self.push(Value::Number(a / b));
                }
                OpCode::Nil => self.push(Value::Nil),
                OpCode::True => self.push(Value::Bool(true)),
                OpCode::False => self.push(Value::Bool(false)),
                OpCode::Not => {
                    let res = is_falsey(&self.pop());

                    self.push(Value::Bool(res))
                }
                OpCode::Equal => {
                    let b = self.pop();
                    let a = self.pop();
                    self.push(Value::Bool(values_equal(&a, &b)));
                }
                OpCode::Greater => {
                    if !self.peek(0).is_number() || !self.peek(1).is_number() {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }

                    let b = *self.pop().as_number().expect("Operands must be numbers.");
                    let a = *self.pop().as_number().expect("Operands must be numbers.");

                    self.push(Value::Bool(a > b));
                }
                OpCode::Less => {
                    if !self.peek(0).is_number() || !self.peek(1).is_number() {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }

                    let b = *self.pop().as_number().expect("Operands must be numbers.");
                    let a = *self.pop().as_number().expect("Operands must be numbers.");

                    self.push(Value::Bool(a < b));
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

    fn peek(&self, idx: usize) -> &Value {
        return self.stack.peek(idx);
    }

    fn runtime_error(&self, msg: &str) {
        eprintln!("{}", msg);
    }
}

fn values_equal(a: &Value, b: &Value) -> bool {
    return a == b;
}

fn is_falsey(v: &Value) -> bool {
    if v.is_nil() {
        return true;
    }

    match v.as_bool() {
        Some(b) => return !(*b),
        None => return false,
    }
}

fn is_truthy(v: &Value) -> bool {
    return !is_falsey(v);
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_is_falsey() {
        assert!(is_falsey(&Value::Bool(false)));
        assert!(is_falsey(&Value::Nil));

        assert!(is_truthy(&Value::Bool(true)));
        assert!(is_truthy(&Value::Number(0.0)));
        assert!(is_truthy(&Value::Number(14427.0)));
    }
}
