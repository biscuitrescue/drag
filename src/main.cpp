#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "./token_maps.hpp"
// #include "./token_maps.hpp"

std::string make_asm(const std::vector<Token> &token_array) { // err here
  std::stringstream output;
  // init global line and status for sstream
  output << "global _start\n_start:\n";

  for (int i = 0; i < token_array.size(); i++) {
    const Token& token = token_array.at(i);
    if (token.type == TokenType::exit) {
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

  Tokeniser tokeniser(std::move(contents));
  std::vector<Token> token_array = tokeniser.tokenise();

  // std::cout << make_asm(token_array) << std::endl;

  {
    std::ofstream file("./out.asm");
    file << make_asm(token_array);
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");

  return EXIT_SUCCESS;
}
