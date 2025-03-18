#pragma once

#include "token_maps.hpp"

namespace node {

  struct Expr { 
    Token int_lit;
  };

  struct Exit {
    Expr expr;
  };
}

class Parser {

  private:
    std::vector<Token> m_tokens;
    size_t m_ind = 0;

    [[nodiscard]] inline std::optional<Token> peak(int n = 1) const {
      if (m_ind + n >= m_tokens.size()) {
        return {};
      } else {
        return m_tokens.at(m_ind);
      }
    }

    inline Token consume() {
      return m_tokens.at(m_ind++);
    }

  public: 
    inline explicit Parser(std::vector<Token> token_array):
      m_tokens(std::move(token_array)) {}

// Expression Parser
    std::optional<node::Expr> parse_expr() { 
      if (peak().has_value() && peak().value().type == TokenType::int_lit) {
        return node::Expr{.int_lit = consume()};
      } else {
        return {};
      }
    }

// Exit Parser
    std::optional<node::Exit> parse() {
      std::optional<node::Exit> node_exit;
      while (peak().has_value()) {
        if (peak().value().type == TokenType::exit) {
          consume();
          if (auto expr = parse_expr()) { // if optional has val -> true else false
            node_exit = node::Exit{.expr = expr.value()};
          } else {
            std::cerr << "invalid expr" << std::endl;
            exit(EXIT_FAILURE);
          }
          if (!peak().has_value() || peak().value().type != TokenType::semicol) {
            std::cerr << "invalid expr" << std::endl;
            exit(EXIT_FAILURE);
          }
        }
      }
      m_ind = 0;
      return node_exit;
    }
};
