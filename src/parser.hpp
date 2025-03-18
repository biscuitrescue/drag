#pragma once

#include <cstdlib>
#include <vector>
#include <stddef.h>

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

    std::optional<node::Exit> parse_expr() { }

    std::optional<node::Exit> parse() {
      std::optional<node::Exit> node_exit;
      while (peak().has_value()) {
        if (peak().value().type == TokenType::exit) {
          if (auto expr = parse_expr()) {

          } else {
            std::cerr << "invalid expr" << std::endl;
            exit(EXIT_FAILURE);
          }

        }
      }
      m_ind = 0;
      return node_exit;
    }
};
