#[derive(PartialEq, Clone)]
pub enum Obj {
    String(ObjString),
}

impl Obj {
    /// Returns `true` if the lox object is [`String`].
    ///
    /// [`String`]: LoxObject::String
    #[must_use]
    pub fn is_string(&self) -> bool {
        return matches!(self, Self::String(..));
    }

    pub fn as_string(&self) -> Option<&ObjString> {
        if let Self::String(v) = self {
            return Some(v);
        } else {
            return None;
        }
    }
}

#[derive(PartialEq, Clone)]
pub struct ObjString {
    pub str: String,
}

pub fn copy_string(string: &str) -> Obj {
    // since string contains also the quotation marks, it must be at least length 2
    assert!(string.len() >= 2);
    assert!(string.chars().next().is_some_and(|c| return c == '"'));
    assert!(string.chars().next_back().is_some_and(|c| return c == '"'));

    let str: String = string.chars().skip(1).take(string.len() - 2).collect();

    return Obj::String(ObjString { str }); // TODO: this should be tracked in the memory manager
}

// pub fn allocate_string(chars: &str, len: usize) -> ObjString {}

// // TODO: Implement something with allocators at some point
// pub fn allocate_object(size: usize) -> Obj {}
