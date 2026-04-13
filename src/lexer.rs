#[derive(Debug, Clone, PartialEq)]
pub enum Token {
    Fn,
    Extern,
    Identifier(String),
    Number(f64),
    Operator(char),
    LParen,
    RParen,
    LBrace,
    RBrace,
    Colon,
    Arrow,
    Comma,
    EOF,
}

pub struct Lexer<'a> {
    input: std::str::Chars<'a>,
    current_char: Option<char>,
}

impl<'a> Lexer<'a> {
    pub fn new(input: &'a str) -> Self {
        let mut lexer = Lexer {
            input: input.chars(),
            current_char: None,
        };
        lexer.advance();
        lexer
    }

    fn advance(&mut self) {
        self.current_char = self.input.next();
    }

    fn skip_whitespace(&mut self) {
        while let Some(c) = self.current_char {
            if c.is_whitespace() {
                self.advance();
            } else {
                break;
            }
        }
    }

    pub fn next_token(&mut self) -> Token {
        self.skip_whitespace();
        
        while let Some(c) = self.current_char {
            if c.is_alphabetic() {
                // Read identifier or keyword
                let mut ident = String::new();
                while let Some(c) = self.current_char {
                    if c.is_alphanumeric() {
                        ident.push(c);
                        self.advance();
                    } else {
                        break;
                    }
                }
                return match ident.as_str() {
                    "fn" => Token::Fn,
                    "extern" => Token::Extern,
                    _ => Token::Identifier(ident),
                };
            }

            if c.is_digit(10) || c == '.' {
                let mut num_str = String::new();
                while let Some(c) = self.current_char {
                    if c.is_digit(10) || c == '.' {
                        num_str.push(c);
                        self.advance();
                    } else {
                        break;
                    }
                }
                return Token::Number(num_str.parse().unwrap_or(0.0));
            }

            // Check for potential operators or comments
            if c == '/' {
                if let Some(next_c) = self.input.clone().next() {
                    if next_c == '/' {
                        // It's a comment
                        self.advance(); // consume first '/'
                        while let Some(c) = self.current_char {
                            if c == '\n' || c == '\r' {
                                break;
                            }
                            self.advance();
                        }
                        self.skip_whitespace();
                        continue;
                    }
                }
            }

            if c == '-' {
                if let Some(next_c) = self.input.clone().next() {
                    if next_c == '>' {
                        self.advance(); // consume '-'
                        self.advance(); // consume '>'
                        return Token::Arrow;
                    }
                }
            }

            let token = match c {
                '(' => Token::LParen,
                ')' => Token::RParen,
                '{' => Token::LBrace,
                '}' => Token::RBrace,
                ':' => Token::Colon,
                ',' => Token::Comma,
                _ => Token::Operator(c),
            };
            self.advance();
            return token;
        }

        Token::EOF
    }
}
