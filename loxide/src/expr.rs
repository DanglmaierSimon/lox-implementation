use crate::{token::Token, value::LoxValue};

#[derive(Debug)]
pub enum Expr {
    Literal {
        value: LoxValue,
    },

    Binary {
        left: Box<Expr>,
        operator: Token,
        right: Box<Expr>,
    },

    Grouping {
        expr: Box<Expr>,
    },

    Unary {
        operator: Token,
        right: Box<Expr>,
    },
}

pub enum Stmt {

}