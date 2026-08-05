#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <string>
#include <cstdlib>
#include <vector>
#include </home/bayayayo/Документи/GitHub/VT/Tokenization.h>


void Asemble(std::vector<Token> tokens) {
    std::ofstream outFile("./built/assembly.asm", std::ios::out);

    outFile << "global _start:" << std::endl;
    outFile << "_start:" << std::endl;
    try {
        if (tokens[0].Type == TokenType::exit && tokens[1].Type == TokenType::int_lit && tokens[2].Type ==
            TokenType::semi) {
            outFile << "\tmov rax, 60" << std::endl;
            outFile << "\t" << "mov rdi, " << tokens[1].Value->c_str() << std::endl;
            outFile << "\tsyscall" << std::endl;
            }
    } catch (std::exception &e) {
        std::cerr << "Bruh! u have not enough Tokens " << std::endl;
    }
    outFile.close();
}


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
    Asemble(Tokens);

    std::system("nasm -felf64 ./built/assembly.asm -o ./built/assembly.o && ld ./built/assembly.o -o ./built/assembly");
    return 0;
}
