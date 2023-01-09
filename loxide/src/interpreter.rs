use crate::{
    expr::Expr,
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

pub struct Interpreter {}

impl Interpreter {
    pub fn interpret(&mut self, expr: &Expr) {
        match evaluate(expr) {
            Ok(value) => println!("{}", value.to_string()),
            Err(e) => println!("{:?}", e),
        }
    }
}

pub fn evaluate(expr: &Expr) -> Result<LoxValue, RuntimeError> {
    match expr {
        Expr::Literal { value } => {
            return Ok(value.clone());
        }
        Expr::Binary {
            left,
            operator,
            right,
        } => {
            let lhs = evaluate(left)?;
            let rhs = evaluate(right)?;

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
        Expr::Grouping { expr } => {
            return evaluate(&expr);
        }
        Expr::Unary { operator, right } => {
            let rhs = evaluate(right)?;

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
    }
}
