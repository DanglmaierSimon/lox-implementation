use crate::interpreter::{Interpreter, RuntimeError};

#[derive(Debug, Clone, PartialEq)]
pub enum LoxValue {
    Nil(),
    Bool(bool),
    Number(f64),
    String(String),
    Function(LoxFunction),
}

impl ToString for LoxValue {
    fn to_string(&self) -> String {
        match &self {
            LoxValue::Nil() => return String::from("nil"),
            LoxValue::Bool(b) => return b.to_string(),
            LoxValue::Number(n) => return n.to_string(),
            LoxValue::String(s) => return s.clone(),
            LoxValue::Function(f) => return "todo".to_owned(),
        }
    }
}

impl LoxValue {
    /// Returns `true` if the lox value is [`Number`].
    ///
    /// [`Number`]: LoxValue::Number
    #[must_use]
    pub fn is_number(&self) -> bool {
        matches!(self, Self::Number(..))
    }

    /// Returns `true` if the lox value is [`Nil`].
    ///
    /// [`Nil`]: LoxValue::Nil
    #[must_use]
    pub fn is_nil(&self) -> bool {
        matches!(self, Self::Nil(..))
    }

    /// Returns `true` if the lox value is [`Bool`].
    ///
    /// [`Bool`]: LoxValue::Bool
    #[must_use]
    pub fn is_bool(&self) -> bool {
        matches!(self, Self::Bool(..))
    }

    pub fn as_bool(&self) -> bool {
        if let Self::Bool(b) = self {
            return *b;
        }
        panic!("Value is not a bool!")
    }

    pub fn as_number(&self) -> f64 {
        if let Self::Number(v) = *self {
            return v;
        }
        panic!("Value is not a number!")
    }

    /// Returns `true` if the lox value is [`String`].
    ///
    /// [`String`]: LoxValue::String
    #[must_use]
    pub fn is_string(&self) -> bool {
        matches!(self, Self::String(..))
    }

    pub fn as_string(&self) -> String {
        if let Self::String(v) = self {
            return v.clone();
        }
        panic!("Value is not a string!")
    }
}

pub trait LoxCallable {
    fn arity(&self) -> usize;
    fn call(
        &self,
        interpreter: &mut Interpreter,
        args: &Vec<LoxValue>,
    ) -> Result<LoxValue, RuntimeError>;
}

#[derive(Debug, Clone, PartialEq)]
pub struct LoxFunction {}
