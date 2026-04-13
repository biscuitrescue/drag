#[derive(Debug, Clone, PartialEq)]
pub enum Expr {
    Number(f64),
    Variable(String),
    Binary {
        op: char,
        left: Box<Expr>,
        right: Box<Expr>,
    },
    Call {
        callee: String,
        args: Vec<Expr>,
    },
}

#[derive(Debug, Clone, PartialEq)]
pub enum Type {
    F64,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Decl {
    pub name: String,
    pub args: Vec<(String, Type)>,
    pub return_type: Type,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Func {
    pub decl: Decl,
    pub body: Expr,
}
