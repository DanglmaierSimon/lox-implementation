use std::collections::HashMap;

use crate::{interpreter::RuntimeError, token::Token, value::LoxValue};

#[derive(Debug, Clone)]
pub struct Environment {
    pub enclosing: Option<Box<Environment>>, // Figure out
    values: HashMap<String, LoxValue>,
}

impl<'a> Environment {
    pub fn new() -> Self {
        Self {
            enclosing: None,
            values: HashMap::new(),
        }
    }

    pub fn new_with_env(enclosing: Environment) -> Self {
        Self {
            enclosing: Some(Box::new(enclosing)),
            values: HashMap::new(),
        }
    }

    pub fn define(&mut self, name: String, value: LoxValue) {
        self.values.insert(name, value);
    }

    pub fn get(&self, name: &Token) -> Result<&LoxValue, RuntimeError> {
        match self.values.get(&name.lexeme) {
            Some(v) => return Ok(v),
            None => match &self.enclosing {
                Some(enc) => return enc.get(name),
                None => {
                    return Err(RuntimeError {
                        token: name.clone(),
                        msg: format!("Undefined variable '{}'.", name.lexeme),
                    })
                }
            },
        }
    }

    #[must_use]
    pub fn assign(&mut self, name: &Token, value: LoxValue) -> Option<RuntimeError> {
        if self.values.contains_key(&name.lexeme) {
            self.values.insert(name.lexeme.clone(), value);
            return None;
        }

        match self.enclosing {
            Some(_) => return self.enclosing.as_mut().unwrap().assign(name, value), // jesus christ: https://stackoverflow.com/questions/69615120/extracting-a-mutable-reference-from-an-option
            None => {
                return Some(RuntimeError {
                    token: name.clone(),
                    msg: format!("Undefined variable '{}'.", name.lexeme),
                })
            }
        }
    }
}
