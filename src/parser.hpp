#pragma once

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
};
