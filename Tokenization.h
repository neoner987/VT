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
    semi,
    open_parent,
    close_parent,
    ident,
    _int,
    eq
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
            case TokenType::open_parent:
                std::cout << "(";
                break;
            case TokenType::close_parent:
                std::cout << ")";
                break;
            case TokenType::semi:
                std::cout << ";\n";
                break;
            case TokenType::_int:
                std::cout << "ціле ";
                break;
            case TokenType::eq:
                std::cout << "= ";
                break;
            case TokenType::ident:
                std::cout << Value->c_str() << " ";
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
    inline explicit Tokenizer(std::string src)
        : m_src(std::move(src)) {
    }

    inline std::vector<Token> Tokenize() {
        std::vector<Token> tokens;
        std::string buffer{};
        while (peek().has_value()) {
            if (peek(1).has_value()) {
                if (isCyrillicLetter(peek().value(), peek(1).value()) && !isdigit(peek().value())) {
                    buffer.push_back(consume());
                    buffer.push_back(consume());
                    while ((isCyrillicLetter(peek().value(), peek(1).value()) || isdigit(peek().value())) && peek().has_value()) {
                        if (isCyrillicLetter(peek().value(), peek(1).value())) {
                            buffer.push_back(consume());
                            buffer.push_back(consume());
                        } else if (isdigit(peek().value()) || isAsciiAlnum(peek().value())) {
                            buffer.push_back(consume());
                        }
                    }
                    if (buffer == "вийти") {
                        tokens.push_back(Token(TokenType::exit));
                        buffer.clear();
                    }
                    else if (buffer == "ціле") {
                        tokens.push_back(Token(TokenType::_int));
                        buffer.clear();
                    }
                    else {
                        tokens.push_back(Token(TokenType::ident, buffer));
                        buffer.clear();
                    }
                }
            }
            if (isdigit(peek().value())) {
                buffer.push_back(consume());
                while (isdigit(peek().value()) && peek().has_value()) {
                    buffer.push_back(consume());
                    if (not peek(1).has_value()) { break; }
                }
                tokens.push_back(Token(TokenType::int_lit, buffer));
                buffer.clear();
            }else if (peek().value() =='(') {
                consume();
                tokens.push_back(Token(TokenType::open_parent));
            }else if (peek().value() ==  ')') {
                consume();
                tokens.push_back(Token(TokenType::close_parent));
            }
            else if (peek().value() ==  '=') {
                consume();
                tokens.push_back(Token(TokenType::eq));
            }else if (peek().value() == ';') {
                tokens.push_back(Token(TokenType::semi));
                consume();
            } else if (isspace(peek().value())) {
                consume();
            } else {
                std::cout << " Bruh! wrong syntacys" << std::endl;
                break;
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    };

    inline char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index = 0;
};