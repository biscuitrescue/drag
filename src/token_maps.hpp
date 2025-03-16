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
  identifier,
  _return,
};

struct Token {
  TokenType type;
  std::optional<std::string> value;
};

const std::unordered_map<std::string, TokenType> keyword_map = {
  {"exit", TokenType::exit},
  {"return", TokenType::_return},
};

class Tokeniser {
  private:

    [[nodiscard]] std::optional<char> peak(int n = 1) const { // nodiscard to warn if not using ret val because no reason not to use ret val from const function
      if (m_ind + n >= m_src.length()) {
        return {};
      } else {
        return m_src.at(m_ind);
      }
    }

    char consume() {
      return m_src.at(m_ind++);
    }

    const std::string m_src;
    int m_ind = 0;

  public:

    inline explicit Tokeniser(const std::string& src)
      : m_src(std::move(src)) {

      }

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

        } else if (std::iswspace(peak().value())) {
          consume();
          continue;
        } else if (peak().has_value()) {
          buf.push_back(consume());
          while(peak().has_value() && std::isdigit(peak().value())) {
            buf.push_back(consume());
          }
          token_array.push_back({.type = TokenType::int_lit, .value = buf});
          buf.clear();
          continue;
        } else if (peak().value() == ';') {
          token_array.push_back({.type = TokenType::semicol, .value = ";"});
        } else {
          std::cerr << "Unexpected identifier" << buf << std::endl;
          exit(EXIT_FAILURE);
        }
      } 

      m_ind = 0;
      return token_array;

    };
};
