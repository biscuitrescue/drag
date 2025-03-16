#pragma once

#include <cctype>
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
  semicol
};

struct Token {
  TokenType type;
  std::optional<std::string> value;

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
          if (buf == "exit") {
            token_array.push_back({.type = TokenType::exit});
            buf.clear();
            continue;
          } else {
            std::cerr << "Unexpected identifier: " << buf << std::endl;
            exit(EXIT_FAILURE);
          }
        } else if (peak().has_value() && std::iswspace(peak().value())) {
          continue;
        } else if (std::isdigit(peak().value())) {
          buf.push_back(consume());
          while(peak().has_value() && std::isdigit(peak().value())) {
            buf.push_back(consume());
          }
          token_array.push_back({.type = TokenType::int_lit});
          buf.clear();
          continue;
        } else if (peak().value() == ';') {
          token_array.push_back({.type = TokenType::semicol});
          buf.clear();
          continue;
        } else {
          std::cerr << "Error tokenising" << std::endl;
          exit(EXIT_FAILURE);
        }
      } 

      m_ind = 0;
      return token_array;

    };
};
