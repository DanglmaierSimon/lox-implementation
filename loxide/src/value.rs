use std::ops::Index;

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum Value {
    Number(f64),
    Bool(bool),
    Nil,
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

    /// Returns `true` if the value is [`Bool`].
    ///
    /// [`Bool`]: Value::Bool
    #[must_use]
    pub fn is_bool(&self) -> bool {
        return matches!(self, Self::Bool(..));
    }

    pub fn as_bool(&self) -> Option<&bool> {
        if let Self::Bool(v) = self {
            return Some(v);
        } else {
            return None;
        }
    }

    /// Returns `true` if the value is [`Nil`].
    ///
    /// [`Nil`]: Value::Nil
    #[must_use]
    pub fn is_nil(&self) -> bool {
        return matches!(self, Self::Nil);
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
