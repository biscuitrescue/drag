# drag

> a minimal compiler built from scratch for learning and experimentation

`drag` is a small, self-contained compiler designed to explore the internals of parsing, ast construction, semantic analysis, and code generation. it’s written in c++ using cmake and aims for clarity over completeness — perfect for education, tinkering, and extending.

---

## ✨ features

- lexer, parser, and basic ast implementation
- minimalistic expression language with rust-like syntax
- type-checking and basic error reporting
- intermediate representation (ir) generation
- easy to read, build, and modify

---

## 🛠️ getting started

### prerequisites

- cmake ≥ 3.12
- c++17 or later
- (optional) llvm or custom backend if you're generating real code

### build instructions


```bash
git clone https://github.com/biscuitrescue/drag.git
cd drag
mkdir build && cd build
cmake ..
make
```
