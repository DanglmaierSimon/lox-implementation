use std::ops::Index;

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum Value {
    Number(f64),
    Nil,
}

impl std::ops::Div for Value {
    type Output = Option<Value>;

    fn div(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Number(lhs), Value::Number(rhs)) => return Some(Value::Number(lhs / rhs)),
            (_, _) => return None,
        }
    }
}

impl std::ops::Mul for Value {
    type Output = Option<Value>;

    fn mul(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Number(lhs), Value::Number(rhs)) => return Some(Value::Number(lhs * rhs)),
            (_, _) => return None,
        }
    }
}

impl std::ops::Sub for Value {
    type Output = Option<Value>;

    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Number(lhs), Value::Number(rhs)) => return Some(Value::Number(lhs - rhs)),
            (_, _) => return None,
        }
    }
}

impl std::ops::Add for Value {
    type Output = Option<Value>;

    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Number(lhs), Value::Number(rhs)) => return Some(Value::Number(lhs + rhs)),
            (_, _) => return None,
        }
    }
}

impl Value {
    /// Returns `true` if the value is [`Number`].
    ///
    /// [`Number`]: Value::Number
    #[must_use]
    pub fn is_number(&self) -> bool {
        return matches!(self, Self::Number(..));
    }

    pub fn as_number(&self) -> Option<&f64> {
        if let Self::Number(v) = self {
            return Some(v);
        } else {
            return None;
        }
    }
}

impl std::ops::Neg for Value {
    type Output = Option<Value>;

    fn neg(self) -> Self::Output {
        if let Some(num) = self.as_number() {
            return Some(Value::Number(-num));
        } else {
            return None;
        }
    }
}

impl Default for Value {
    fn default() -> Self {
        return Self::Nil;
    }
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
