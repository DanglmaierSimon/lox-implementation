use crate::token::Token;
use crate::{expr::*, value::LoxValue};

pub struct AstPrinter {}

impl AstPrinter {
    fn parenthesise(&self, name: &String, exprs: &[&Box<Expr>]) -> String {
        let mut retval = String::new();

        retval.push('(');
        retval.push_str(name);

        for expr in exprs {
            retval.push(' ');
            retval.push_str(&accept(self, expr));
        }

        retval.push(')');

        return retval;
    }

    pub fn print_expr(&mut self, expr: Expr) {
        let str = accept(self, &expr);
        println!("{}", str)
    }
}

impl ExprVisitor<String> for AstPrinter {
    fn visit_binary_expr(&self, left: &Box<Expr>, operator: &Token, right: &Box<Expr>) -> String {
        return self.parenthesise(&operator.lexeme, &[&left, &right]);
    }

    fn visit_literal_expr(&self, literal: &LoxValue) -> String {
        match literal {
            LoxValue::Nil() => "nil".to_string(),
            LoxValue::Bool(b) => b.to_string(),
            LoxValue::Number(n) => n.to_string(),
            LoxValue::String(str) => str.to_string(),
        }
    }

    fn visit_unary_expr(&self, operator: &Token, right: &Box<Expr>) -> String {
        return self.parenthesise(&operator.lexeme, &[right]);
    }

    fn visit_grouping_expr(&self, expr: &Box<Expr>) -> String {
        return self.parenthesise(&"group".to_owned(), &[expr]);
    }
}
