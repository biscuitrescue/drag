use crate::ast::{Decl, Expr, Func, Type};
use crate::lexer::{Lexer, Token};

pub struct Parser<'a> {
    lexer: Lexer<'a>,
    current_token: Token,
}

impl<'a> Parser<'a> {
    pub fn new(mut lexer: Lexer<'a>) -> Self {
        let current_token = lexer.next_token();
        Parser {
            lexer,
            current_token,
        }
    }

    fn advance(&mut self) {
        self.current_token = self.lexer.next_token();
    }

    pub fn parse_number_expr(&mut self) -> Result<Expr, String> {
        if let Token::Number(val) = self.current_token {
            self.advance();
            Ok(Expr::Number(val))
        } else {
            Err("Expected number".to_string())
        }
    }

    pub fn parse_paren_expr(&mut self) -> Result<Expr, String> {
        self.advance();
        let expr = self.parse_expression()?;
        if self.current_token != Token::RParen {
            return Err("Expected ')'".to_string());
        }
        self.advance();
        Ok(expr)
    }

    pub fn parse_identifier_expr(&mut self) -> Result<Expr, String> {
        let id_name = if let Token::Identifier(ref name) = self.current_token {
            name.clone()
        } else {
            return Err("Expected identifier".to_string());
        };
        self.advance();

        if self.current_token != Token::LParen {
            return Ok(Expr::Variable(id_name));
        }
        // function call
        self.advance();
        let mut args = Vec::new();
        if self.current_token != Token::RParen {
            loop {
                args.push(self.parse_expression()?);
                if self.current_token == Token::RParen {
                    break;
                }
                if self.current_token != Token::Comma {
                    return Err("Expected ')' or ',' in argument list".to_string());
                }
                self.advance();
            }
        }
        self.advance();

        Ok(Expr::Call {
            callee: id_name,
            args,
        })
    }

    pub fn parse_primary(&mut self) -> Result<Expr, String> {
        match self.current_token {
            Token::Identifier(_) => self.parse_identifier_expr(),
            Token::Number(_) => self.parse_number_expr(),
            Token::LParen => self.parse_paren_expr(),
            _ => Err(format!(
                "Unknown token when expecting an expression: {:?}",
                self.current_token
            )),
        }
    }

    fn get_tok_precedence(token: &Token) -> i32 {
        if let Token::Operator(op) = token {
            match op {
                '<' => 10,
                '+' => 20,
                '-' => 20,
                '*' => 40,
                '/' => 40,
                _ => -1,
            }
        } else {
            -1
        }
    }

    pub fn parse_bin_op_rhs(&mut self, expr_prec: i32, mut lhs: Expr) -> Result<Expr, String> {
        loop {
            let tok_prec = Self::get_tok_precedence(&self.current_token);
            if tok_prec < expr_prec {
                return Ok(lhs);
            }

            let op = if let Token::Operator(op_char) = self.current_token {
                op_char
            } else {
                return Err("Expected operator".to_string());
            };
            self.advance();

            let mut rhs = self.parse_primary()?;

            let next_prec = Self::get_tok_precedence(&self.current_token);
            if tok_prec < next_prec {
                rhs = self.parse_bin_op_rhs(tok_prec + 1, rhs)?;
            }

            lhs = Expr::Binary {
                op,
                left: Box::new(lhs),
                right: Box::new(rhs),
            };
        }
    }

    pub fn parse_expression(&mut self) -> Result<Expr, String> {
        let lhs = self.parse_primary()?;
        self.parse_bin_op_rhs(0, lhs)
    }

    fn parse_type(&mut self) -> Result<Type, String> {
        if let Token::Identifier(ref type_name) = self.current_token {
            let typ = match type_name.as_str() {
                "f64" => Type::F64,
                _ => return Err(format!("Unknown type: {}", type_name)),
            };
            self.advance();
            Ok(typ)
        } else {
            Err("Expected type identifier".to_string())
        }
    }

    pub fn parse_decl(&mut self) -> Result<Decl, String> {
        let name = if let Token::Identifier(ref n) = self.current_token {
            n.clone()
        } else {
            return Err("Expected function name in prototype".to_string());
        };
        self.advance();

        if self.current_token != Token::LParen {
            return Err("Expected '(' in prototype".to_string());
        }
        self.advance();

        let mut args = Vec::new();
        if self.current_token != Token::RParen {
            loop {
                let arg_name = if let Token::Identifier(ref arg) = self.current_token {
                    arg.clone()
                } else {
                    return Err("Expected argument name".to_string());
                };
                self.advance();

                if self.current_token != Token::Colon {
                    return Err("Expected ':' after argument name".to_string());
                }
                self.advance();

                let arg_type = self.parse_type()?;
                args.push((arg_name, arg_type));

                if self.current_token == Token::Comma {
                    self.advance();
                } else {
                    break;
                }
            }
        }

        if self.current_token != Token::RParen {
            return Err("Expected ')' in prototype".to_string());
        }
        self.advance();

        if self.current_token != Token::Arrow {
            return Err("Expected '->' after prototype".to_string());
        }
        self.advance();

        let return_type = self.parse_type()?;

        Ok(Decl { name, args, return_type })
    }

    pub fn parse_definition(&mut self) -> Result<Func, String> {
        if self.current_token != Token::Fn {
            return Err("Expected 'fn' keyword".to_string());
        }
        self.advance();
        let proto = self.parse_decl()?;
        
        if self.current_token != Token::LBrace {
            return Err("Expected '{' in function body".to_string());
        }
        self.advance();
        
        let body = self.parse_expression()?;
        
        if self.current_token != Token::RBrace {
            return Err("Expected '}' at end of function body".to_string());
        }
        self.advance();
        
        Ok(Func { decl: proto, body })
    }

    pub fn parse_extern(&mut self) -> Result<Decl, String> {
        if self.current_token != Token::Extern {
            return Err("Expected 'extern' keyword".to_string());
        }
        self.advance();
        self.parse_decl()
    }

    pub fn parse_top_level_expr(&mut self) -> Result<Func, String> {
        let body = self.parse_expression()?;
        let proto = Decl {
            name: "".to_string(), // anonymous function
            args: Vec::new(),
            return_type: Type::F64,
        };
        Ok(Func { decl: proto, body })
    }
}
