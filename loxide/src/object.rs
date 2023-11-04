use std::{
    collections::hash_map::DefaultHasher,
    hash::{Hash, Hasher},
};

use crate::gc::MemoryManager;

#[derive(Clone)]
pub enum Obj {
    String(ObjString),
}

impl PartialEq for Obj {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::String(lhs), Self::String(rhs)) => return lhs == rhs,
        }
    }
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

// TODO: Properly implement the hash function and store it inline in the ObjString struct
#[derive(PartialEq, Clone, Hash, Eq)]
pub struct ObjString {
    str: String,
    hash: u64,
}

impl ObjString {
    pub fn new(str: &str) -> Self {
        return Self {
            str: str.to_string(),
            hash: hash_string(str),
        };
    }

    pub fn str(&self) -> &str {
        return self.str.as_ref();
    }

    pub fn hash(&self) -> u64 {
        return self.hash;
    }
}

pub fn copy_string(gc: &MemoryManager, string: &str) -> ObjString {
    return allocate_string(gc, string); // TODO: this should be tracked in the memory manager
}

pub fn allocate_string(gc: &MemoryManager, chars: &str) -> ObjString {
    return ObjString::new(chars);
}

// // TODO: Implement something with allocators at some point
pub fn allocate_object(gc: &mut MemoryManager, size: usize) -> Obj {
    todo!("GC and object allocation is still to be implemented");
}

fn hash_string(chars: &str) -> u64 {
    let mut s = DefaultHasher::new();
    chars.hash(&mut s);
    return s.finish();
}
