#pragma once
#include <iostream>
#include <vector>
#include <optional>


inline bool isAscii(const unsigned char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

inline bool isCyrillicLetter(const unsigned char b1, const unsigned char b2) {
    if (b1 == 0xD0 && (b2 >= 0x80 && b2 <= 0xBF)) return true;
    if (b1 == 0xD1 && (b2 >= 0x80 && b2 <= 0xBF)) return true;
    return false;
}


enum class TokenType {
    exit,
    int_lit,
    semi,
    open_parent,
    close_parent,
    ident,
    _int,
    eq,
    plus,
    minus,
    mult
};


struct Token {
    TokenType Type = {};
    std::optional<std::string> Value = {};

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
                        if ((isCyrillicLetter(peek().value(), peek(1).value())) ){
                            buffer.push_back(consume());
                            buffer.push_back(consume());
                        } else if (isdigit(peek().value()) || isAscii(peek().value())) {
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
            }else if (peek().value() ==  '+') {
                consume();
                tokens.push_back(Token(TokenType::plus));
            }else if (peek().value() ==  '*') {
                consume();
                tokens.push_back(Token(TokenType::mult));
            }else if (peek().value() ==  '-') {
                consume();
                tokens.push_back(Token(TokenType::minus));
            } else if (isspace(peek().value())) {
                consume();
            } else {
                std::cout << " Bruh! wrong syntax" << std::endl;
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