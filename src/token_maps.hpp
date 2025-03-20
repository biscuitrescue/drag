#pragma once

#include <cctype>
#include <unordered_map>
#include <cstdlib>
#include <cwctype>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

enum class TokenType {
  exit,
  int_lit,
  semicol,
  open_paren,
  close_paren,
  identifier,
  _return,
};

struct Token {
  TokenType type;
  std::optional<std::string> value;

  // For easier debugging 
  std::string tokenTypeToString() const {
    switch (this->type) {
      case TokenType::semicol: return "Semicolon";
      case TokenType::int_lit: return "Integer Literal";
      case TokenType::exit: return "Exit";
      case TokenType::open_paren: return "(";
      case TokenType::close_paren: return ")";
      default: return "Unknown";
    }
  }
};

const std::unordered_map<std::string, TokenType> keyword_map = {
  {"exit", TokenType::exit},
  {"return", TokenType::_return},
};

class Tokeniser {
  private:

    [[nodiscard]] inline std::optional<char> peak(int offset = 0) const { // nodiscard to warn if not using ret val because no reason not to use ret val from const function
      if (m_ind + offset >= m_src.length()) {
        return {};
      } else {
        return m_src.at(m_ind + offset);
      }
    }

    inline char consume() {
      return m_src.at(m_ind++);
    }

    const std::string m_src;
    int m_ind = 0;

  public:

    inline explicit Tokeniser(const std::string& src)
      : m_src(std::move(src)) {}

    inline std::vector<Token> tokenise() {

      std::vector<Token> token_array;
      std::string buf;


      while(peak().has_value()) {
        if (isalpha(peak().value())) {
          buf.push_back(consume());
          while (peak().has_value() && std::isalnum(peak().value())) {
            buf.push_back(consume());
          }
          auto it = keyword_map.find(buf);
          if (it != keyword_map.end()) {
            token_array.push_back({.type = it->second, .value = buf});
          } else {
            token_array.push_back({.type = TokenType::identifier, .value = buf});
          }

          buf.clear();
          continue;

        } else if (peak().value() == '(') {
          consume();
          token_array.push_back({.type = TokenType::open_paren});
          continue;
        } else if (peak().value() == ')') {
          consume();
          token_array.push_back({.type = TokenType::close_paren});
          continue;
        } else if (std::iswspace(peak().value())) {
          consume();
          continue;
        } else if (std::isdigit(peak().value())) {
          buf.push_back(consume());
          while (peak().has_value() && std::isdigit(peak().value())) {
            buf.push_back(consume());
          }
          token_array.push_back({.type = TokenType::int_lit, .value = buf});

          buf.clear();
          continue;
        } else if (peak().value() == ';') {
          consume();
          token_array.push_back({.type = TokenType::semicol, .value = ";"});
          continue;
        } else {
          std::cerr << "Unexpected identifier" << buf << std::endl;
          exit(EXIT_FAILURE);
        }
      } 

      m_ind = 0;
      return token_array;

    };
};
