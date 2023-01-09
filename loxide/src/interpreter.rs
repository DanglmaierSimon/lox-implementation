use crate::{
    expr::{accept, Expr, ExprVisitor},
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
        let evaluator = ExprEvaluator {};

        match evaluator.evaluate(expr) {
            Ok(value) => println!("{}", value.to_string()),
            Err(e) => println!("{:?}", e),
        }
    }
}

pub struct ExprEvaluator {}

impl ExprEvaluator {
    fn evaluate(&self, expr: &Expr) -> Result<LoxValue, RuntimeError> {
        return accept(self, expr);
    }
}

impl ExprVisitor<Result<LoxValue, RuntimeError>> for ExprEvaluator {
    fn visit_binary_expr(
        &self,
        left: &Box<Expr>,
        operator: &Token,
        right: &Box<Expr>,
    ) -> Result<LoxValue, RuntimeError> {
        let lhs = self.evaluate(left)?;
        let rhs = self.evaluate(right)?;

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

    fn visit_literal_expr(&self, literal: &LoxValue) -> Result<LoxValue, RuntimeError> {
        return Ok(literal.clone());
    }

    fn visit_unary_expr(
        &self,
        operator: &Token,
        right: &Box<Expr>,
    ) -> Result<LoxValue, RuntimeError> {
        let rhs = self.evaluate(right)?;

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

    fn visit_grouping_expr(&self, expr: &Box<Expr>) -> Result<LoxValue, RuntimeError> {
        return self.evaluate(&expr);
    }
}
