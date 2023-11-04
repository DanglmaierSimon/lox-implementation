use std::rc::Rc;

use crate::{
    chunk::Chunk,
    compiler::compile,
    debug::{disassemble_instruction, print_value},
    gc::MemoryManager,
    object::{Obj, ObjString},
    opcode::OpCode,
    scanner::Scanner,
    stack::Stack,
    table::Table,
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
    gc: MemoryManager,
    globals: Table,
}

impl VM {
    pub fn new() -> Self {
        return Self {
            chunk: Chunk::new(),
            ip: 0,
            stack: Stack::new(),
            gc: MemoryManager {
                strings: Table::new(),
            },
            globals: Table::new(),
        };
    }

    pub fn interpret(&mut self, source: Rc<String>) -> InterpretResult {
        let scanner = Scanner::new(Rc::clone(&source));

        let compile_result = compile(scanner, &self.gc);

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

            disassemble_instruction(self.ip, *instruction, &self.chunk);

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
                    return InterpretResult::Ok;
                }
                OpCode::Constant(idx) => {
                    let constant = self.chunk.constants()[*idx].clone();
                    self.push(constant);
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
                    if self.peek(0).is_string() && self.peek(1).is_string() {
                        self.concatenate();
                    } else if self.peek(0).is_number() && self.peek(1).is_number() {
                        let b = *self.pop().as_number().expect("Operands must be numbers.");
                        let a = *self.pop().as_number().expect("Operands must be numbers.");

                        self.push(Value::Number(a + b));
                    } else {
                        self.runtime_error("Operands must be numbers.");
                        return InterpretResult::RuntimeError;
                    }
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
                OpCode::Pop => {
                    self.pop();
                }
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
                OpCode::Print => {
                    print_value(&self.pop());
                    println!();
                }
                OpCode::DefineGlobal(index) => {
                    let name = self.read_string(*index).clone();
                    self.globals.set(&name, self.peek(0).clone());
                    self.pop();
                }
                OpCode::GetGlobal(index) => {
                    let name = self.read_string(*index);
                    let value = self.globals.get(name);
                    match value {
                        Some(v) => {
                            self.push(v.clone());
                        }
                        None => {
                            self.runtime_error(&format!("Undefined variable '{}'.", name.str()));
                            return InterpretResult::RuntimeError;
                        }
                    }
                }
            }
        }
    }

    fn read_string(&self, index: usize) -> &ObjString {
        return self.chunk.constants()[index]
            .as_obj()
            .expect("Must be object.")
            .as_string()
            .expect("mus be objString");
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

    fn concatenate(&mut self) {
        let tempb = self.pop();

        let b = tempb
            .as_obj()
            .expect("Should be an object")
            .as_string()
            .expect("Should be a string");

        let tempa = self.pop();
        let a = tempa
            .as_obj()
            .expect("Should be an object")
            .as_string()
            .expect("Should be a string");

        let mut concatted: String = a.str().clone().to_string();
        concatted.push_str(b.str());

        let objs = ObjString::new(&concatted);

        self.push(Value::Obj(Box::new(Obj::String(objs))))
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
