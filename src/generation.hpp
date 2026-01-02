#pragma once

#include <sstream>

#include "parser.hpp"

class Generator {
  private:
    const node::Stmt m_root;

  public:
    inline Generator(node::Stmt root)
      : m_root(std::move(root)) {}

    [[nodiscard]] std::string generate() const {
      std::stringstream output;
      output << "global _start\n_start:\n";
      output << "    mov rax, 60\n";
      // USE holds_alternative<t>(variant);
      // output << "    mov rdi, " << m_root.expr.int_lit.value.value() << "\n";
      // output << "   mov rdi, " << std::get<>(m_root) << "\n";
      output << "    syscall";// terminate
      return output.str();
    }

  private:
};
