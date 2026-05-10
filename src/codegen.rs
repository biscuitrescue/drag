use inkwell::FloatPredicate;
use inkwell::OptimizationLevel;
use inkwell::builder::Builder;
use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::passes::PassBuilderOptions;
use inkwell::targets::{CodeModel, InitializationConfig, RelocMode, Target, TargetMachine};
use inkwell::types::BasicMetadataTypeEnum;
use inkwell::values::{BasicValueEnum, FunctionValue};
use std::collections::HashMap;

use crate::ast::{Decl, Expr, Func, Type};

pub struct Compiler<'a, 'ctx> {
    pub context: &'ctx Context,
    pub builder: &'a Builder<'ctx>,
    pub module: &'a Module<'ctx>,
    pub fn_value_opt: Option<FunctionValue<'ctx>>,
    pub variables: HashMap<String, BasicValueEnum<'ctx>>,
}

impl<'a, 'ctx> Compiler<'a, 'ctx> {
    pub fn optimize_module(&self) -> Result<(), String> {
        Target::initialize_all(&InitializationConfig::default());

        let target_triple = TargetMachine::get_default_triple();
        let target = Target::from_triple(&target_triple).map_err(|e| e.to_string())?;
        let target_machine = target
            .create_target_machine(
                &target_triple,
                "generic",
                "",
                OptimizationLevel::Default,
                RelocMode::PIC,
                CodeModel::Default,
            )
            .ok_or_else(|| "Failed to create target machine".to_string())?;

        let passes = ["instcombine", "reassociate", "gvn", "adce", "simplifycfg"];
        self.module
            .run_passes(
                &passes.join(","),
                &target_machine,
                PassBuilderOptions::create(),
            )
            .map_err(|e| e.to_string())?;
        Ok(())
    }

    pub fn ensure_entrypoint(&self) {
        if self.module.get_function("main").is_some() {
            return;
        }

        let main_ty = self.context.i32_type().fn_type(&[], false);
        let main_fn = self.module.add_function("main", main_ty, None);
        let entry = self.context.append_basic_block(main_fn, "entry");
        self.builder.position_at_end(entry);
        self.builder
            .build_return(Some(&self.context.i32_type().const_zero()))
            .unwrap();
    }

    pub fn compile_expr(&mut self, expr: &Expr) -> Result<BasicValueEnum<'ctx>, &'static str> {
        match expr {
            Expr::Number(val) => Ok(self.context.f64_type().const_float(*val).into()),
            Expr::Variable(name) => match self.variables.get(name) {
                Some(var) => Ok(*var),
                None => Err("Unknown variable name"),
            },
            Expr::Binary { op, left, right } => {
                let lhs = self.compile_expr(left)?.into_float_value();
                let rhs = self.compile_expr(right)?.into_float_value();
                match op {
                    '+' => Ok(self
                        .builder
                        .build_float_add(lhs, rhs, "addtmp")
                        .unwrap()
                        .into()),
                    '-' => Ok(self
                        .builder
                        .build_float_sub(lhs, rhs, "subtmp")
                        .unwrap()
                        .into()),
                    '*' => Ok(self
                        .builder
                        .build_float_mul(lhs, rhs, "multmp")
                        .unwrap()
                        .into()),
                    '/' => Ok(self
                        .builder
                        .build_float_div(lhs, rhs, "divtmp")
                        .unwrap()
                        .into()),
                    '<' => {
                        let cmp = self
                            .builder
                            .build_float_compare(FloatPredicate::ULT, lhs, rhs, "cmptmp")
                            .unwrap();
                        Ok(self
                            .builder
                            .build_unsigned_int_to_float(cmp, self.context.f64_type(), "booltmp")
                            .unwrap()
                            .into())
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
                    inkwell::values::ValueKind::Basic(val) => Ok(val.into()),
                    _ => Err("Func call returned void"),
                }
            }
        }
    }

    pub fn compile_decl(&self, proto: &Decl) -> Result<FunctionValue<'ctx>, &'static str> {
        let args_types = proto
            .args
            .iter()
            .map(|(_, t)| match t {
                Type::F64 => self.context.f64_type().into(),
            })
            .collect::<Vec<BasicMetadataTypeEnum>>();
        let args_types = args_types.as_slice();

        let fn_type = match proto.return_type {
            Type::F64 => self.context.f64_type().fn_type(args_types, false),
        };
        let fn_val = self.module.add_function(proto.name.as_str(), fn_type, None);

        // set arguments names
        for (i, arg) in fn_val.get_param_iter().enumerate() {
            if let inkwell::values::BasicValueEnum::FloatValue(fv) = arg {
                fv.set_name(proto.args[i].0.as_str());
            }
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
            let arg_name = function.decl.args[i].0.as_str();
            self.variables.insert(arg_name.to_string(), arg);
        }

        let body = self.compile_expr(&function.body)?;
        self.builder.build_return(Some(&body)).unwrap();

        if function_val.verify(true) {
            Ok(function_val)
        } else {
            unsafe {
                function_val.delete();
            }
            Err("Invalid generated function.")
        }
    }
}
