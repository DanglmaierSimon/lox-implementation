use crate::{expr::*, value::LoxValue};

fn parenthesise(name: &String, exprs: &[&Box<Expr>]) -> String {
    let mut retval = String::new();

    retval.push('(');
    retval.push_str(name);

    for expr in exprs {
        retval.push(' ');
        retval.push_str(&print_expr(expr));
    }

    retval.push(')');

    return retval;
}

pub fn print_expr(expr: &Expr) -> String {
    match expr {
        Expr::Literal { value } => match value {
            LoxValue::Nil() => "nil".to_string(),
            LoxValue::Bool(b) => b.to_string(),
            LoxValue::Number(n) => n.to_string(),
            LoxValue::String(str) => str.to_string(),
        },
        Expr::Binary {
            left,
            operator,
            right,
        } => parenthesise(&operator.lexeme, &[&left, &right]),
        Expr::Grouping { expr } => parenthesise(&"group".to_owned(), &[expr]),
        Expr::Unary { operator, right } => parenthesise(&operator.lexeme, &[right]),
    }
}
