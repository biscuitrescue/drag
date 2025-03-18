#pragma once

#include <cstdlib>
#include <vector>
#include "token_maps.hpp"

struct NodeExpr { Token int_lit; };

struct NodeExit {
  NodeExpr expr;
};


class Parser {
  private:
    const std::vector<Token> m_tokens;
    size_t m_ind = 0;

    [[nodiscard]] inline std::optional<Token> peak(int n = 1) const { // nodiscard to warn if not using ret val because no reason not to use ret val from const function
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
    inline explicit Parser(std::vector<Token> token_arr)
      : m_tokens(std::move(token_arr)) {}

    std::optional<NodeExpr> parse_expr() {
      if (peak().has_value() && peak()->type == TokenType::int_lit) {
        return NodeExpr{.int_lit = consume()};
      } else {
        return {};
      }
    }

    std::optional<NodeExit> parse() {
      std::optional<NodeExit> exit_node;

      while (peak().has_value()) {
        if (peak().value().type == TokenType::exit) {
          consume();
          if (auto node_expr = parse_expr()) {
            exit_node = NodeExit{.expr = node_expr.value()};
          } else {
            std::cerr << "Invalid expression 1" << std::endl;
            exit(EXIT_FAILURE);
          }

          if(peak().has_value() && peak()->type == TokenType::semicol) {
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
