mod ast;
mod lexer;
mod parser;
mod codegen;

use clap::Parser;
use inkwell::context::Context;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Arg {
    /// String to compile
    #[arg(short, long)]
    input: Option<String>,
}

fn main() {
    let args = Arg::parse();

    let code = if let Some(ref input) = args.input {
        input.clone()
    } else {
        println!("Please provide input using --input");
        return;
    };

    println!("Compiling: {}", code);

    let lexer = lexer::Lexer::new(&code);
    let mut parser = parser::Parser::new(lexer);

    let ast_opt = parser.parse_definition().or_else(|_| parser.parse_top_level_expr());
    
    match ast_opt {
        Ok(ast) => {
            println!("Parsed AST: {:#?}", ast);

            let context = Context::create();
            let module = context.create_module("repl");
            let builder = context.create_builder();

            let mut compiler = codegen::Compiler {
                context: &context,
                builder: &builder,
                module: &module,
                fn_value_opt: None,
                variables: std::collections::HashMap::new(),
            };

            match compiler.compile_fn(&ast) {
                Ok(_) => {
                    println!("\nLLVM IR Generated Successfully!");
                    println!("{}", module.print_to_string().to_string());
                }
                Err(e) => {
                    eprintln!("Failed to generate LLVM IR: {}", e);
                }
            }
        }
        Err(err) => {
            eprintln!("Parser error: {}", err);
        }
    }
}
