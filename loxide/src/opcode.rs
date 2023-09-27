#[derive(Debug, PartialEq)]
pub enum OpCode {
    Return,
    Constant(usize), // TODO: Deviation from reference impl
}

impl OpCode {
    pub fn as_constant(&self) -> Option<&usize> {
        if let Self::Constant(v) = self {
            return Some(v);
        } else {
            return None;
        }
    }

    /// Returns `true` if the op code is [`Constant`].
    ///
    /// [`Constant`]: OpCode::Constant
    #[must_use]
    pub fn is_constant(&self) -> bool {
        return matches!(self, Self::Constant(..));
    }
}
