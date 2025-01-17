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
  std::optional<std::string> value;

};

std::vector<Token> tokenise(const std::string& str) {
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

      if (buf == "return") {
        token_array.push_back({.type = TokenType::_return});
        buf.clear();
        continue;
      } else {
        std::cerr << "Error ret" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (std::iswspace(c)) {
      // buf.clear();
      continue;
    } else if (std::isdigit(c)) {
      buf.push_back(c);
      i++;
      while(i < str.length() && std::isdigit(str.at(i))) { // Shouldnt be i????
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
}

std::string make_asm(const std::vector<Token> &token_array) {
  std::stringstream output;
  // init global line and status for sstream
  output << "global _start\n_start:\n";

  for (int i = 0; i < token_array.size(); i++) {
    const Token& token = token_array.at(i);
    if (token.type == TokenType::_return) {
      if (i + 1 < token_array.size() && token_array.at(i + 1).type == TokenType::int_lit) {
        if (i + 2 < token_array.size() && token_array.at(i + 2).type  == TokenType::semicol) {
          output << "    mov rax, 60\n";
          output << "    mov rdi, " << token_array.at(i + 1).value.value() << "\n";
          output << "    syscall";// terminate
        }
      }
    } 
  }
  return output.str();
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

  std::vector<Token> token_array = tokenise(contents);

  // std::cout << make_asm(token_array) << std::endl;

  {
    std::ofstream file("./out.asm");
    file << make_asm(token_array);
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");

  return EXIT_SUCCESS;
}
