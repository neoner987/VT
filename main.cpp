#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <string>
#include <cstdlib>
#include <vector>
#include "Tokenization.h"
#include "Parser.h"
#include "Generation.h"




int main() {
    std::string filePath = "./Проєкт.VT";

    std::string text;
    {
        std::fstream file;
        file.open(filePath, std::ios::in);
        std::stringstream strStream;
        strStream << file.rdbuf();
        text = strStream.str();
    }

    Tokenizer tokenizer(std::move(text));
    std::vector<Token> Tokens = tokenizer.Tokenize();
    Parser parser(Tokens);
    if (auto prog = parser.parse_prog()) {
        Generator generator(prog.value());
        std::ofstream output_file("./built/output.asm");
        output_file << generator.gen_prog().str();
        output_file.close();
    }
    else {
        std::cout << "Boob";
    }


    std::system("nasm -felf64 ./built/output.asm -o ./built/output.o && ld ./built/output.o -o ./built/output");
    return 0;
}
