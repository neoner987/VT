#pragma once
#include <iostream>
#include <vector>
#include <optional>


bool isAsciiAlnum(unsigned char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

bool isCyrillicLetter(unsigned char b1, unsigned char b2) {
    int codepoint = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
    return (codepoint >= 0x0400 && codepoint <= 0x04FF);
}


enum class TokenType {
    exit,
    int_lit,
    semi
};


struct Token {
    TokenType Type;
    std::optional<std::string> Value = {};

    void Print() {
        switch (Type) {
            case TokenType::exit:
                std::cout << "вийти ";
                break;
            case TokenType::int_lit:
                std::cout << Value->c_str();
                break;
            case TokenType::semi:
                std::cout << ";\n";
                break;
        }
    }

    Token() = default;

    explicit Token(TokenType type, std::string value)
        : Type(type), Value(std::move(value)) {
    }

    explicit Token(TokenType type)
        : Type(type) {
    }
};


class Tokenizer {
public:
    inline explicit Tokenizer(const std::string src)
        : m_src(std::move(src)) {
    }

    inline std::vector<Token> Tokenize() {
        std::vector<Token> tokens;
        std::string buffer{};
        while (peak().has_value()) {
            if (peak(1).has_value()) {
                if (isCyrillicLetter(peak().value(), peak(1).value()) && !isdigit(peak().value())) {
                    buffer.push_back(consume());
                    buffer.push_back(consume());
                    while ((isCyrillicLetter(peak().value(), peak(1).value()) || isdigit(peak().value())) && peak().
                           has_value()) {
                        if (isCyrillicLetter(peak().value(), peak(1).value())) {
                            buffer.push_back(consume());
                            buffer.push_back(consume());
                        } else if (isdigit(peak().value()) || isAsciiAlnum(peak().value())) {
                            buffer.push_back(consume());
                        }
                    }
                    if (buffer == "вийти") {
                        tokens.push_back(Token(TokenType::exit));
                        buffer.clear();
                    } else {
                        std::cout << " Bruh! Wrone Token." << std::endl;
                        break;
                    }
                }
            }
            if (isdigit(peak().value())) {
                buffer.push_back(consume());
                while (isdigit(peak().value()) && peak().has_value()) {
                    buffer.push_back(consume());
                    if (not peak(1).has_value()) { break; }
                }
                tokens.push_back(Token(TokenType::int_lit, buffer));
                buffer.clear();
            } else if (peak().value() == ';') {
                tokens.push_back(Token(TokenType::semi));
                consume();
            } else if (isspace(peak().value())) {
                consume();
            } else {
                std::cout << " Bruh! wrone syntacys" << std::endl;
                break;
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] std::optional<char> peak(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    };

    char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    int m_index = 0;
};