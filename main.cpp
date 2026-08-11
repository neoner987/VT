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
    if (auto exit_node = parser.parse()) {
        Generation generation(exit_node.value(), "./built/assembly.asm");
        generation.generate_assembly();
    }
    else {
        std::cout << "Boob";
    }


    std::system("nasm -felf64 ./built/assembly.asm -o ./built/assembly.o && ld ./built/assembly.o -o ./built/assembly");
    return 0;
}
