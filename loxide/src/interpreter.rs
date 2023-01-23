use crate::{
    environment::Environment,
    expr::{self},
    token::{Token, TokenType},
    value::LoxValue,
};

fn is_truthy(value: &LoxValue) -> bool {
    return !is_falsey(value);
}

fn is_falsey(value: &LoxValue) -> bool {
    return value.is_nil() || (value.is_bool() && !value.as_bool());
}

fn is_equal(lhs: &LoxValue, rhs: &LoxValue) -> bool {
    return lhs == rhs;
}

#[derive(Debug)]
pub struct RuntimeError {
    pub token: Token,
    pub msg: String,
}

#[derive(Debug)]
pub struct Interpreter {
    env: Environment,
}

impl Interpreter {
    pub fn new() -> Self {
        Self {
            env: Environment::new(),
        }
    }

    pub fn interpret(&mut self, stmts: Vec<expr::Stmt>) -> Option<RuntimeError> {
        for s in stmts {
            match self.execute(s) {
                Some(err) => return Some(err),
                None => {}
            }
        }

        return None;
    }

    // TODO: can we consume the epxressions and statements here?
    fn execute(&mut self, stmt: expr::Stmt) -> Option<RuntimeError> {
        match stmt {
            expr::Stmt::Expression { expr } => {
                _ = self.evaluate(expr);
                return None;
            }
            expr::Stmt::Print { expr } => match self.evaluate(expr) {
                Ok(val) => {
                    println!("{}", val.to_string());
                    return None;
                }
                Err(e) => return Some(e),
            },
            expr::Stmt::Variable { name, initializer } => {
                let value = match initializer {
                    Some(expr) => match self.evaluate(expr) {
                        Ok(v) => v,
                        Err(e) => return Some(e),
                    },
                    None => LoxValue::Nil(),
                };

                self.env.define(name.lexeme, value);
                return None;
            }
            expr::Stmt::Block { statements } => {
                return self.execute_block(statements, Environment::new_with_env(self.env.clone()));
            }
        }
    }

    // TODO: can we consume the epxressions and statements here?
    fn evaluate(&mut self, expr: expr::Expr) -> Result<LoxValue, RuntimeError> {
        match expr {
            expr::Expr::Literal { value } => {
                return Ok(value.clone());
            }
            expr::Expr::Binary {
                left,
                operator,
                right,
            } => {
                let lhs = self.evaluate(*left)?;
                let rhs = self.evaluate(*right)?;

                match operator.token_type {
                    TokenType::Minus => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Number(lhs.as_number() - rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::Slash => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Number(lhs.as_number() / rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::Star => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Number(lhs.as_number() * rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::Plus => {
                        if rhs.is_number() && lhs.is_number() {
                            return Ok(LoxValue::Number(lhs.as_number() + rhs.as_number()));
                        }

                        if lhs.is_string() && rhs.is_string() {
                            let mut str = lhs.as_string();
                            str.push_str(&rhs.as_string());
                            return Ok(LoxValue::String(str));
                        }

                        return Err(RuntimeError {
                            token: operator.clone(),
                            msg: "Operands must be two numbers or two strings.".to_owned(),
                        });
                    }
                    TokenType::Greater => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Bool(lhs.as_number() > rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::GreaterEqual => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Bool(lhs.as_number() >= rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::Less => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Bool(lhs.as_number() < rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::LessEqual => match lhs.is_number() && rhs.is_number() {
                        true => return Ok(LoxValue::Bool(lhs.as_number() <= rhs.as_number())),
                        false => {
                            return Err(RuntimeError {
                                token: operator.clone(),
                                msg: "Operands must be numbers.".to_owned(),
                            })
                        }
                    },
                    TokenType::BangEqual => return Ok(LoxValue::Bool(!is_equal(&lhs, &rhs))),
                    TokenType::EqualEqual => return Ok(LoxValue::Bool(is_equal(&lhs, &rhs))),
                    _ => {}
                };

                unreachable!()
            }
            expr::Expr::Grouping { expr } => {
                return self.evaluate(*expr);
            }
            expr::Expr::Unary { operator, right } => {
                let rhs = self.evaluate(*right)?;

                if !rhs.is_number() {
                    return Err(RuntimeError {
                        token: operator.clone(),
                        msg: "Operand must be a number.".to_owned(),
                    });
                }

                match operator.token_type {
                    TokenType::Minus => match rhs.is_number() {
                        true => return Ok(LoxValue::Number(-rhs.as_number())),
                        false => unimplemented!(),
                    },
                    TokenType::Bang => {
                        return Ok(LoxValue::Bool(!is_truthy(&rhs)));
                    }
                    _ => unreachable!("This should never be reached!"),
                }
            }
            expr::Expr::Variable { name } => return self.env.get(&name).cloned(),
            expr::Expr::Assignment { name, value } => {
                let v = self.evaluate(*value)?;
                match self.env.assign(&name, v.clone()) {
                    None => return Ok(v),
                    Some(err) => return Err(err),
                }
            }
        }
    }

    fn execute_block(
        &mut self,
        statements: Vec<Box<expr::Stmt>>,
        env: Environment,
    ) -> Option<RuntimeError> {
        self.env = env;

        for stmt in statements {
            match self.execute(*stmt) {
                Some(err) => {
                    if let Some(enclosing) = self.env.enclosing.clone() {
                        self.env = *enclosing
                    } else {
                        panic!("already in the topmost environment");
                    }
                    return Some(err);
                }
                None => {}
            }
        }

        if let Some(enclosing) = self.env.enclosing.clone() {
            self.env = *enclosing
        } else {
            panic!("already in the topmost environment");
        }

        return None;
    }
}
