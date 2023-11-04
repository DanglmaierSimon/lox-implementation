#[derive(Debug, PartialEq, Clone, Copy)]
pub enum OpCode {
    Return,
    Constant(usize), // TODO: Deviation from reference impl
    Negate,

    Add,
    Subtract,
    Multiply,
    Divide,

    Nil,
    True,
    False,

    Not,

    Equal,
    Greater,
    Less,

    Print,
    Pop, // for expression statements

    DefineGlobal(usize),
    GetGlobal(usize),
}
