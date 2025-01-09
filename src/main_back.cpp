#include <cctype>
#include <cstdlib>
#include <cwctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class TokenType {
  _return,
  int_lit,
  semicol
};

struct Token {
  TokenType type;
  std:: optional<std::string> value;

};

std::vector<Token> tokenise(const std::string& str) {
  std::vector<Token> token_array;

  std::string buf;

  for (int i = 0; i < str.length(); i++) {
    char c = str.at(i);
    if (std::isalpha(c)) {
      buf.push_back(c);
      i++;
      while (std::isalnum(c)) {
        buf.push_back(str.at(i));
        i++;
      }
      i--;

      if (buf == "return") {
        token_array.push_back({.type = TokenType::_return});
        buf.clear();
        continue;
      } else {
        std::cerr << "Error" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (std::iswspace(c)) {
      continue;
    } else if (std::isdigit(c)) {

    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Need a source file" << std::endl;
    std::cerr << "drag <input file>" << std::endl;
    return EXIT_FAILURE;
  }

  std::stringstream content_stream;
  std::string contents;
  {
    std::ifstream input (argv[1]); // alternative to fstream with std::ios::in 
    content_stream << input.rdbuf();
    contents = content_stream.str();
  }

  tokenise(contents);

  return EXIT_SUCCESS;
}
