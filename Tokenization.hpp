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
    star,
    fslash,
    open_curly,
    close_curly,
    _if

};

inline std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::star:
        case TokenType::fslash:
            return 1;
        default:
            return {};
    }
}


struct Token {
    TokenType Type = {};
    std::optional<std::string> Value = {};

    Token() = default;

    explicit Token(const TokenType type, const std::string &value)
        : Type(type), Value(value) {
    }

    explicit Token(const TokenType type)
        : Type(type) {
    }
};


class Tokenizer {
public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src)) {
    }

    std::vector<Token> Tokenize() {
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
                    else if (buffer == "якщо") {
                        tokens.push_back(Token(TokenType::_if));
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
                if (peek().has_value() && peek().value() == '*' && peek(1).has_value() && peek(1).value() == '*') {
                    consume();
                    consume();
                    while (peek().has_value() && peek().value() != '\n')
                        {
                        consume();
                    }
                    continue;
                }
                tokens.push_back(Token(TokenType::star));
            }else if (peek().value() ==  '-') {
                consume();
                tokens.push_back(Token(TokenType::minus));
            }else if (peek().value() ==  '/') {
                consume();
                if ( peek().has_value() && peek().value() == '*') {
                    consume();
                    while (peek().has_value() && peek().value() != '*') {
                        consume();
                    }
                    if (!peek().has_value()) {
                        std::cout << " Bruh! Comment never ended! " << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    consume();
                    consume();
                    continue;
                }
                tokens.push_back(Token(TokenType::fslash));
            }else if (peek().value() ==  '{') {
                consume();
                tokens.push_back(Token(TokenType::open_curly));
            }else if (peek().value() ==  '}') {
                consume();
                tokens.push_back(Token(TokenType::close_curly));
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
    [[nodiscard]] std::optional<char> peek(const int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        return m_src.at(m_index + offset);
    };

    char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index = 0;
};