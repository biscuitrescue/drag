use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::types::BasicMetadataTypeEnum;
use inkwell::values::{FloatValue, FunctionValue};
use inkwell::FloatPredicate;
use std::collections::HashMap;

use crate::ast::{Decl, Expr, Func};

pub struct Compiler<'a, 'ctx> {
    pub context: &'ctx Context,
    pub builder: &'a Builder<'ctx>,
    pub module: &'a Module<'ctx>,
    pub fn_value_opt: Option<FunctionValue<'ctx>>,
    pub variables: HashMap<String, FloatValue<'ctx>>,
}

impl<'a, 'ctx> Compiler<'a, 'ctx> {
    pub fn compile_expr(&mut self, expr: &Expr) -> Result<FloatValue<'ctx>, &'static str> {
        match expr {
            Expr::Number(val) => Ok(self.context.f64_type().const_float(*val)),
            Expr::Variable(name) => match self.variables.get(name) {
                Some(var) => Ok(*var),
                None => Err("Unknown variable name"),
            },
            Expr::Binary { op, left, right } => {
                let lhs = self.compile_expr(left)?;
                let rhs = self.compile_expr(right)?;
                match op {
                    '+' => Ok(self.builder.build_float_add(lhs, rhs, "addtmp").unwrap()),
                    '-' => Ok(self.builder.build_float_sub(lhs, rhs, "subtmp").unwrap()),
                    '*' => Ok(self.builder.build_float_mul(lhs, rhs, "multmp").unwrap()),
                    '/' => Ok(self.builder.build_float_div(lhs, rhs, "divtmp").unwrap()),
                    '<' => {
                        let cmp = self
                            .builder
                            .build_float_compare(FloatPredicate::ULT, lhs, rhs, "cmptmp")
                            .unwrap();
                        Ok(self
                            .builder
                            .build_unsigned_int_to_float(cmp, self.context.f64_type(), "booltmp")
                            .unwrap())
                    }
                    _ => Err("Invalid binary operator"),
                }
            }
            Expr::Call { callee, args } => {
                let function = self
                    .module
                    .get_function(callee)
                    .ok_or("Unknown function referenced")?;
                if function.count_params() as usize != args.len() {
                    return Err("Incorrect # arguments passed");
                }

                let mut compiled_args = Vec::with_capacity(args.len());
                for arg in args {
                    compiled_args.push(self.compile_expr(arg)?.into());
                }

                let result = self
                    .builder
                    .build_call(function, &compiled_args, "calltmp")
                    .unwrap();
                match result.try_as_basic_value() {
                    inkwell::values::ValueKind::Basic(val) => Ok(val.into_float_value()),
                    _ => Err("Func call returned void"),
                }
            }
        }
    }

    pub fn compile_decl(&self, proto: &Decl) -> Result<FunctionValue<'ctx>, &'static str> {
        let f64_type = self.context.f64_type();
        let args_types = std::iter::repeat(f64_type)
            .take(proto.args.len())
            .map(|f| f.into())
            .collect::<Vec<BasicMetadataTypeEnum>>();
        let args_types = args_types.as_slice();

        let fn_type = self.context.f64_type().fn_type(args_types, false);
        let fn_val = self.module.add_function(proto.name.as_str(), fn_type, None);

        // set arguments names
        for (i, arg) in fn_val.get_param_iter().enumerate() {
            arg.into_float_value().set_name(proto.args[i].as_str());
        }

        Ok(fn_val)
    }

    pub fn compile_fn(&mut self, function: &Func) -> Result<FunctionValue<'ctx>, &'static str> {
        let function_val = self.compile_decl(&function.decl)?;
        let entry = self.context.append_basic_block(function_val, "entry");

        self.builder.position_at_end(entry);

        self.fn_value_opt = Some(function_val);

        self.variables.reserve(function.decl.args.len());
        for (i, arg) in function_val.get_param_iter().enumerate() {
            let arg_name = function.decl.args[i].as_str();
            self.variables
                .insert(arg_name.to_string(), arg.into_float_value());
        }

        let body = self.compile_expr(&function.body)?;
        self.builder.build_return(Some(&body)).unwrap();

        if function_val.verify(true) {
            Ok(function_val)
        } else {
            // Unsafe clean up
            unsafe {
                function_val.delete();
            }
            Err("Invalid generated function.")
        }
    }
}
