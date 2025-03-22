#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <vector>
#include "token_maps.hpp"

namespace  node {
  struct Expr { Token int_lit; };

  struct Stmt { std::vector<Token> tokens;};

  struct Exit {
    node::Expr expr;
  };
}


class Parser {
  private:
    const std::vector<Token> m_tokens;
    size_t m_ind = 0;

    [[nodiscard]] inline std::optional<Token> peak(int offset = 0) const { // nodiscard to warn if not using ret val because no reason not to use ret val from const function
      if (m_ind + offset >= m_tokens.size()) {
        return {};
      }
      return m_tokens.at(m_ind + offset);
    }


    inline Token consume() {
      return m_tokens.at(m_ind++);
    }

  public:
    inline explicit Parser(std::vector<Token> token_arr)
      : m_tokens(std::move(token_arr)) {}

    std::optional<node::Expr> parse_expr() {
      if (peak().has_value() && peak()->type == TokenType::int_lit) {
        return node::Expr{.int_lit = consume()};
      } else {
        return {};
      }
    }

    std::optional<node::Exit> parse() {
      std::optional<node::Exit> exit_node;
      size_t prev_ind = m_ind;

      while (peak().has_value()) {
        if (peak().value().type == TokenType::exit && peak(1).has_value() && peak(1)->type == TokenType::open_paren) {

          consume();
          consume();

          // Evaluate expr
          if (auto node_expr = parse_expr()) {
            exit_node = node::Exit{.expr = node_expr.value()};
          } else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
          }

          // check for )
          if (peak().has_value() && peak()->type == TokenType::close_paren) {
            consume();
          } else {
            std::cerr << "Expected )" << std::endl;
            exit(EXIT_FAILURE);
          }

          // check for semicol
          if (peak().has_value() && peak().value().type == TokenType::semicol) {
            consume();
          } else {
            std::cerr << "semi col not found" << std::endl;
            exit(EXIT_FAILURE);
          }
        }
      }

      m_ind = 0;
      return exit_node;
    }
};
