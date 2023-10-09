pub struct Stack<T, const SIZE: usize> {
    data: [T; SIZE], // TODO: Change to MAybeUninit or use unsafe code
    count: usize,
}

impl<T: std::default::Default + std::marker::Copy, const SIZE: usize> Stack<T, SIZE> {
    pub fn new() -> Self {
        return Self {
            data: [T::default(); SIZE],
            count: 0,
        };
    }

    pub fn count(&self) -> usize {
        return self.count;
    }

    pub fn capacity(&self) -> usize {
        return SIZE;
    }

    pub fn push(&mut self, val: T) {
        assert!(self.count() < SIZE);
        self.data[self.count] = val;
        self.count += 1;
    }

    pub fn pop(&mut self) -> T {
        assert!(!self.empty());
        self.count -= 1;
        return self.data[self.count];
    }

    pub fn top(&self) -> &T {
        return &self.data[self.count() - 1];
    }

    pub fn empty(&self) -> bool {
        return self.count() == 0;
    }

    pub fn data(&self) -> &[T; SIZE] {
        return &self.data;
    }

    pub fn peek(&self, idx: usize) -> &T {
        return &self.data[self.count()-1-idx]
    }
}
