use std::ops::Index;

#[derive(Debug)]
pub enum Value {
    Double(f64),
}

#[derive(Debug)]
pub struct ValueArray {
    values: Vec<Value>,
}

impl Index<usize> for ValueArray {
    type Output = Value;

    fn index(&self, index: usize) -> &Self::Output {
        return &self.values[index];
    }
}

impl ValueArray {
    pub fn new() -> Self {
        return Self { values: Vec::new() };
    }

    pub fn count(&self) -> usize {
        return self.values.len();
    }

    pub fn capacity(&self) -> usize {
        return self.values.capacity();
    }

    pub fn write(&mut self, val: Value) {
        self.values.push(val);
    }
}
