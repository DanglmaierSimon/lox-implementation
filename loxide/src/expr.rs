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
    Variable {
        name: Token,
    },
    Assignment {
        name: Token,
        value: Box<Expr>,
    },
    Logical {
        left: Box<Expr>,
        operator: Token,
        right: Box<Expr>,
    },
}

impl Expr {
    /// Returns `true` if the expr is [`Variable`].
    ///
    /// [`Variable`]: Expr::Variable
    #[must_use]
    pub fn is_variable(&self) -> bool {
        matches!(self, Self::Variable { .. })
    }

    pub fn as_variable(&self) -> Option<Token> {
        if let Self::Variable { name } = self {
            Some(name.clone())
        } else {
            None
        }
    }
}

#[derive(Debug)]
pub enum Stmt {
    Expression {
        expr: Expr,
    },
    Print {
        expr: Expr,
    },
    Variable {
        name: Token,
        initializer: Option<Expr>,
    },
    Block {
        statements: Vec<Stmt>,
    },
    If {
        condition: Expr,
        then_branch: Box<Stmt>,
        else_branch: Option<Box<Stmt>>,
    },
    While {
        condition: Expr,
        body: Box<Stmt>,
    }
}
