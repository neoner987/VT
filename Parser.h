#pragma once
#include <variant>

#include "Tokenization.h"


namespace Node {

    struct NodeExprIntLit {
        Token int_lit;
    };

    struct NodeExprIdent {
        Token ident;
    };

    struct NodeExpr {
        std::variant<NodeExprIdent, NodeExprIntLit> expr;
    };


    struct NodeStmtExit {
        NodeExpr expr;
    };
    struct NodeStmtInt {
        Token ident;
        NodeExpr expr;
    };

    struct NodeStmt {
        std::variant<NodeStmtExit, NodeStmtInt> var;
    };

    struct NodeProg {
        std::vector<NodeStmt> stmts;
    };

}


class Parser {

public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)){

    }



    std::optional<Node::NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().Type == TokenType::int_lit) {
            return Node::NodeExpr {
                .expr = Node::NodeExprIntLit{ .int_lit = consume() }
            };
        }
        else if (peek().has_value() && peek().value().Type == TokenType::ident) {
            if (peek().value().Type == TokenType::ident) {
                return Node::NodeExpr { .expr = Node::NodeExprIdent{ .ident = consume() } };
            }
        }
        return {};

    }

    std::optional<Node::NodeStmt> parse_stmt() {
        std::optional<Node::NodeStmt> stmt_node;
        if (peek().has_value() && peek().value().Type == TokenType::exit) {
            if (peek().value().Type == TokenType::exit) {
                consume();
                if (peek().value().Type == TokenType::open_parent) {
                    consume();
                    if (auto node_expr = parse_expr()) {
                        stmt_node = Node::NodeStmt{ .var = Node::NodeStmtExit { .expr = node_expr.value() }};
                        if (peek().has_value() && peek().value().Type == TokenType::close_parent) {
                            consume();
                            if (!peek().has_value() || peek().value().Type != TokenType::semi) {
                                std::cout << "Bruh! No semi after exit T_T ";
                                exit(EXIT_FAILURE);
                            }
                            else {
                                consume();
                            }
                        }
                        else {
                            std::cout << "Bruh! No open parenthesis.";
                        }
                    }
                    else {
                        std::cout << "Bruh! Invalid expr after exit.";
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    std::cout << "Bruh! No open parenthesis.";
                }
            }
        }
        else if (peek().has_value() && peek().value().Type == TokenType::_int) {
            consume();
            if (peek().value().Type == TokenType::ident) {
                Token ident = consume();
                if (peek().value().Type == TokenType::eq) {
                    consume();
                    if (auto node_expr = parse_expr()) {
                        if (peek().value().Type == TokenType::semi) {
                            consume();
                            stmt_node = Node::NodeStmt{
                                .var = Node::NodeStmtInt{
                                    .ident = ident,
                                    .expr = node_expr.value()
                                }
                            };
                        }
                        else {
                            std::cout << "Bruh! No semi.";
                            exit(EXIT_FAILURE);
                        }
                    }
                    else {
                        std::cout << "Bruh! No value after ident T_T ";
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    std::cout << "Bruh! No = after ident T_T ";
                    exit(EXIT_FAILURE);
                }
            }
            else {
                std::cout << "Bruh! Invalid or no ident.";
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Bruh! Invalid stmt";
            exit(EXIT_FAILURE);
        }
        return stmt_node;
    }

    std::optional<Node::NodeProg> parse_prog() {
        std::optional<Node::NodeProg> prog_nodes;
        std::vector<Node::NodeStmt>stmts;
        while (peek().has_value()) {
            if (auto node_stmt = parse_stmt()) {
                stmts.push_back(node_stmt.value());
            }
            else {
                std::cout << "Bruh! Invalid stmt T_T ";
                exit(EXIT_FAILURE);
            }
        }
        prog_nodes = Node::NodeProg{ .stmts = stmts };
        return prog_nodes;
    }



private:
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens[m_index + offset];

    };

    inline Token consume() {
        return m_tokens[m_index++];
    }
    size_t m_index = 0;
    const std::vector<Token> m_tokens;
};

