#[derive(Debug)]
pub enum LoxValue {
    Nil(),
    Bool(bool),
    Number(f64),
    String(String)
}
