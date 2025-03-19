#include <cstdlib>
#include <fstream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include "./generation.hpp"


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

  Parser parser(std::move(token_array));
  std::optional<node::Exit> tree = parser.parse();

  if (!tree.has_value()) {
    std::cerr << "No exit statement" << std::endl;
    exit(EXIT_FAILURE);
  }

  Generator generator(tree.value());

  {
    std::ofstream file("./out.asm");
    file << generator.generate();
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");

  return EXIT_SUCCESS;
}
