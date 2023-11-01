use std::collections::HashMap;

use crate::{object::ObjString, value::Value};

pub struct Table {
    table: HashMap<ObjString, Value>,
}

impl Table {
    pub fn new() -> Self {
        return Self {
            table: HashMap::new(),
        };
    }

    pub fn find_entry(&self, key: &ObjString) -> Option<&Value> {
        return self.table.get(key);
    }

    pub fn set(&mut self, key: &ObjString, val: Value) -> bool {
        let is_new = self.table.contains_key(key);

        self.table.insert(key.clone(), val);
        return is_new;
    }

    pub fn add_all(&mut self, other: &Table) {
        for (key, val) in &other.table {
            let existing = self.find_entry(key);

            if existing.is_some() {
                self.set(key, val.clone());
            }
        }
    }

    pub fn delete(&mut self, key: &ObjString) -> bool {
        if self.table.is_empty() {
            return false;
        }

        self.table.remove(key);
        return true;
    }
}
