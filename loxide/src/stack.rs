pub struct Stack<T, const SIZE: usize> {
    data: Vec<T>,
}

impl<T: Clone, const SIZE: usize> Stack<T, SIZE> {
    pub fn new() -> Self {
        return Self { data: vec![] };
    }

    pub fn count(&self) -> usize {
        return self.data.len();
    }

    pub const fn capacity(&self) -> usize {
        return SIZE;
    }

    pub fn push(&mut self, val: T) {
        assert!(self.count() < SIZE);
        self.data.push(val);
    }

    pub fn pop(&mut self) -> T {
        assert!(!self.empty());
        assert!(self.data.last().is_some());

        return self.data.pop().unwrap();
    }

    pub fn top(&self) -> &T {
        assert!(!self.empty());
        assert!(self.data.last().is_some());
        self.check_invariants();
        return self.data.last().unwrap();
    }

    pub fn empty(&self) -> bool {
        self.check_invariants();
        return self.count() == 0;
    }

    pub fn data(&self) -> &[T] {
        return self.data.as_slice();
    }

    pub fn peek(&self, idx: usize) -> &T {
        assert!(!self.empty());
        assert!(self.count() > idx);
        self.check_invariants();
        return self.data.iter().rev().nth(idx).unwrap();
    }

    fn check_invariants(&self) {
        // size of the vector may never exceed SIZE
        assert!(self.data.len() <= self.capacity());
    }
}
