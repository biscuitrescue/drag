mod ast;
mod codegen;
mod lexer;
mod parser;

use clap::Parser;
use inkwell::context::Context;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Arg {
    /// String to compile
    #[arg(short, long)]
    input: Option<String>,

    /// File to compile
    #[arg(short, long)]
    file: Option<String>,

    /// Output file
    #[arg(short, long, default_value = "output.ll")]
    out: String,
}

fn main() {
    let args = Arg::parse();

    let code = if let Some(ref file_path) = args.file {
        match std::fs::read_to_string(file_path) {
            Ok(content) => content,
            Err(e) => {
                eprintln!("Failed to read file '{}': {}", file_path, e);
                return;
            }
        }
    } else if let Some(ref input) = args.input {
        input.clone()
    } else {
        println!("Please provide input string (-i) or file (-f)");
        return;
    };

    let lexer = lexer::Lexer::new(&code);
    let mut parser = parser::Parser::new(lexer);

    let ast_opt = parser
        .parse_definition()
        .or_else(|_| parser.parse_top_level_expr());

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
                    compiler.ensure_entrypoint();
                    if let Err(e) = compiler.optimize_module() {
                        eprintln!("Failed to optimize LLVM IR: {}", e);
                        return;
                    }
                    let ir_string = module.print_to_string().to_string();
                    let out_path = args.out;
                    if let Err(e) = std::fs::write(&out_path, ir_string) {
                        eprintln!("Failed to write IR to file '{}': {}", out_path, e);
                    }
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
