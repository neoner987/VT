#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <string>
#include <cstdlib>
#include <vector>
#include "Tokenization.hpp"
#include "Parser.hpp"
#include "Generation.hpp"




int main(int argc, char* argv[]) {


    std::string text;
    {
        std::fstream file;
        file.open(argv[1], std::ios::in);
        std::stringstream strStream;
        strStream << file.rdbuf();
        text = strStream.str();
    }

    Tokenizer tokenizer(text);
    std::vector<Token> Tokens = tokenizer.Tokenize();
    Parser parser(Tokens);
    if (const auto prog = parser.parse_prog()) {
        Generator generator(prog.value());
        std::ofstream output_file("./built/output.asm");
        output_file << generator.gen_prog().str();
        output_file.close();
    }
    else {
        std::cout << "parsing failure";
    }


    std::system("nasm -felf64 ./built/output.asm -o ./built/output.o && ld ./built/output.o -o ./built/output");
    std::system("./built/output; echo $?");
    return 0;
}
