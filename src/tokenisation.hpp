#pragma once

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
    const std::string m_src;
  public:
    inline Tokeniser(const std::string& src)
      : m_src(std::move(src)) {

      }
    inline std::vector<Token> tokenise() {
      std::vector<Token> token_array;

      std::string buf;

      for (int i = 0; i < str.length(); i++) {
        char c = str.at(i);
        if (std::isalpha(c)) {
          buf.push_back(c);
          i++;
          while (i < str.length() && std::isalnum(str.at(i))) {
            buf.push_back(str.at(i));
            i++;
          }
          i--;

          if (buf == "exit") {
            token_array.push_back({.type = TokenType::exit});
            buf.clear();
            continue;
          } else {
            // std::cerr << "Error ret" << buf << c << std::endl;
            std::cerr << "Unexpected identifier: " << buf << std::endl;
            exit(EXIT_FAILURE);
          }
        } else if (std::iswspace(c)) {
          // buf.clear();
          continue;
        } else if (std::isdigit(c)) {
          buf.push_back(c);
          i++;
          while(i < str.length() && std::isdigit(str.at(i))) { // Shouldnt be i???? -> -> // was right
            c = str.at(i);
            buf.push_back(c);
            i++;
          }
          i--;
          token_array.push_back({.type = TokenType::int_lit, .value = buf});
          buf.clear();
        } else if (c == ';') {
          token_array.push_back({.type = TokenType::semicol});
        } else {
          std::cerr << "Error dig" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
      return token_array;

    };
};
