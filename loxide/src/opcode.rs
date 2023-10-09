#[derive(Debug, PartialEq)]
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
}
