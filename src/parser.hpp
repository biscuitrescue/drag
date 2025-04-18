#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <variant>
#include <vector>

#include "token_maps.hpp"

namespace node {

  struct ExprIntLit {
    Token int_lit;
  };

  struct ExprIdent {
    Token ident;
  };

  struct Expr {
    std::variant<ExprIdent, ExprIntLit> var;
  };

  struct StmtExit {
    node::Expr expr;
  };

  struct StmtLet {
    Token ident;
    node::Expr expr;
  };

  struct Stmt {
    std::variant<StmtExit, StmtLet> var;
  };

  struct Prog {
    std::vector<Stmt> stmts;
  };
} // namespace node

class Parser {
  private:
    const std::vector<Token> m_tokens;
    size_t m_ind = 0;

    [[nodiscard]] inline std::optional<Token> peak(int offset = 0)
      const { // nodiscard to warn if not using ret val because no reason not to
              // use ret val from const function
        if (m_ind + offset >= m_tokens.size()) {
          return {};
        }
        return m_tokens.at(m_ind + offset);
      }

    inline Token consume() { return m_tokens.at(m_ind++); }

  public:
    inline explicit Parser(std::vector<Token> token_arr)
      : m_tokens(std::move(token_arr)) {}

    std::optional<node::Expr> parse_expr() {
      if (peak().has_value() && peak()->type == TokenType::int_lit) {
        return node::Expr{.var = node::ExprIntLit{.int_lit = consume()}};
      } else if (peak().has_value() && peak()->type == TokenType::ident) {
        return node::Expr{.var = node::ExprIdent{.ident = consume()}};
      } else {
        return {};
      }
    }

    std::optional<node::Stmt> parse_stmt() {
      if (peak().value().type == TokenType::exit && peak(1).has_value() &&
          peak(1)->type == TokenType::open_paren) {

        consume();
        consume();

        node::StmtExit stmt_exit;

        // Evaluate expr
        if (auto node_expr = parse_expr()) {
          stmt_exit = node::StmtExit{.expr = node_expr.value()};
        } else {
          std::cerr << "Invalid expression" << std::endl;
          exit(EXIT_FAILURE);
        }

        // check for )
        if (peak().has_value() && peak()->type == TokenType::close_paren) {
          consume();
        } else {
          std::cerr << "Expected ')'" << std::endl;
          exit(EXIT_FAILURE);
        }

        // check for semicol
        if (peak().has_value() && peak().value().type == TokenType::semicol) {
          consume();
        } else {
          std::cerr << "Expected ';'" << std::endl;
          exit(EXIT_FAILURE);
        }

        return node::Stmt{.var = stmt_exit};

      } else if (peak().has_value() && peak()->type == TokenType::let && peak(1).has_value() && peak(1)->type == TokenType::ident && peak(2).has_value() && peak(2)->type == TokenType::assign) {
        consume();
        auto stmt_let = node::StmtLet {.ident = consume()};
        consume();

        if (auto expr = parse_expr()) {
          stmt_let.expr = expr.value();
        } else {
          std::cerr << "Invalid expression in let stmt" << std::endl;
          exit(EXIT_FAILURE);
        }

        if (peak().has_value() && peak()->type == TokenType::semicol) {
          consume();
        } else {
          std::cerr << "Expected ';' at end of statement let" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
    }
};
