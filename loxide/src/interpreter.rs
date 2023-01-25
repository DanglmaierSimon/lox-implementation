use crate::{
    environment::Environment,
    expr::{self},
    token::{Token, TokenType},
    value::LoxValue,
};

fn is_truthy(value: &LoxValue) -> bool {
    if value.is_nil() {
        return false;
    }

    if value.is_bool() {
        return value.as_bool();
    }

    return true;
}

fn is_falsey(value: &LoxValue) -> bool {
    return !is_truthy(value);
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

    pub fn interpret(&mut self, stmts: Vec<expr::Stmt>) -> Result<(), RuntimeError> {
        for s in stmts {
            self.execute(&s)?;
        }

        return Ok(());
    }

    // TODO: can we consume the epxressions and statements here?
    fn execute(&mut self, stmt: &expr::Stmt) -> Result<(), RuntimeError> {
        // dbg!(stmt);

        match stmt {
            expr::Stmt::Expression { expr } => {
                let _unused: LoxValue = self.evaluate(expr)?;
                Ok(())
            }
            expr::Stmt::Print { expr } => {
                let value = self.evaluate(expr)?;
                println!("{}", value.to_string());
                Ok(())
            }
            expr::Stmt::Variable { name, initializer } => {
                let value = match initializer {
                    Some(expr) => self.evaluate(expr)?,
                    None => LoxValue::Nil(),
                };

                self.env.define(name.lexeme.clone(), value);
                Ok(())
            }
            expr::Stmt::Block { statements } => {
                self.execute_block(statements, Environment::new_with_env(self.env.clone()))
            }
            expr::Stmt::If {
                condition,
                then_branch,
                else_branch,
            } => {
                let value = self.evaluate(condition)?;
                if is_truthy(&value) {
                    self.execute(&*then_branch)
                } else if let Some(els) = else_branch {
                    self.execute(&*els)
                } else {
                    Ok(())
                }
            }
            expr::Stmt::While { condition, body } => {
                while is_truthy(&self.evaluate(condition)?) {
                    self.execute(&*body)?;
                }

                Ok(())
            }
        }
    }

    // TODO: can we consume the epxressions and statements here?
    fn evaluate(&mut self, expr: &expr::Expr) -> Result<LoxValue, RuntimeError> {
        //dbg!(expr);

        match expr {
            expr::Expr::Literal { value } => {
                return Ok(value.clone());
            }
            expr::Expr::Binary {
                left,
                operator,
                right,
            } => {
                let lhs = self.evaluate(&*left)?;
                let rhs = self.evaluate(&*right)?;

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
                        true => {
                            return Ok(LoxValue::Bool(lhs.as_number() < rhs.as_number()));
                        }
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
                return self.evaluate(&*expr);
            }
            expr::Expr::Unary { operator, right } => {
                let rhs = self.evaluate(&*right)?;

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
                let v = self.evaluate(&*value)?;
                self.env.assign(&name, v.clone())?;
                Ok(v)
            }
            expr::Expr::Logical {
                left,
                operator,
                right,
            } => {
                let left = self.evaluate(&*left)?;

                if operator.token_type == TokenType::Or {
                    if is_truthy(&left) {
                        return Ok(left);
                    }
                } else {
                    if is_falsey(&left) {
                        return Ok(left);
                    }
                }

                return self.evaluate(&*right);
            }
        }
    }

    fn execute_block(
        &mut self,
        statements: &Vec<expr::Stmt>,
        env: Environment,
    ) -> Result<(), RuntimeError> {
        self.env = env;

        for stmt in statements {
            if let Err(err) = self.execute(&*stmt) {
                if let Some(enclosing) = self.env.enclosing.clone() {
                    self.env = *enclosing;
                } else {
                    panic!("already in the topmost environment");
                }

                return Err(err);
            }
        }

        if let Some(enclosing) = self.env.enclosing.clone() {
            self.env = *enclosing;
        } else {
            panic!("already in the topmost environment");
        }

        return Ok(());
    }
}
