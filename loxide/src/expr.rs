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

pub fn accept<T, E: ExprVisitor<T>>(visitor: &E, expr: &Expr) -> T {
    match expr {
        Expr::Binary {
            left,
            operator,
            right,
        } => visitor.visit_binary_expr(left, operator, right),
        Expr::Literal { value } => visitor.visit_literal_expr(value),
        Expr::Grouping { expr } => visitor.visit_grouping_expr(expr),
        Expr::Unary { operator, right } => visitor.visit_unary_expr(operator, right),
    }
}

pub trait ExprVisitor<T> {
    fn visit_binary_expr(&self, left: &Box<Expr>, operator: &Token, right: &Box<Expr>) -> T;
    fn visit_literal_expr(&self, literal: &LoxValue) -> T;
    fn visit_unary_expr(&self, operator: &Token, right: &Box<Expr>) -> T;
    fn visit_grouping_expr(&self, expr: &Box<Expr>) -> T;

    // fn visit_assign_expr(expr: Assign) -> T;

    // fn visit_call_expr(expr: Call) -> T;
    // fn visit_get_expr(expr: Get) -> T;

    // fn visit_logical_expr(expr: Logical) -> T;
    // fn visit_set_expr(expr: Set) -> T;
    // fn visit_super_expr(expr: Super) -> T;
    // fn visit_this_expr(expr: This) -> T;

    // fn visit_variable_expr(expr: Variable) -> T;
}


pub enum Stmt {
    
}