#pragma once
#include <variant>

#include "arena.hpp"
#include "Tokenization.hpp"


struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;


struct NodeBinExprMult {
    NodeExpr* lhs;
    NodeExpr* rhs;

};

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMult*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> var ;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};
struct NodeStmtInt {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtInt*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};


class Parser {

public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4){

    }

    std::optional<NodeTerm*> parse_term() {
        if (peek().has_value() && peek().value().Type == TokenType::int_lit) {
            auto node_expr_int_lit = m_allocator.alloc<NodeTermIntLit>();
            node_expr_int_lit->int_lit = consume();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = node_expr_int_lit;
            return term;
        }
        else if (peek().has_value() && peek().value().Type == TokenType::ident) {
            if (peek().value().Type == TokenType::ident) {
                auto node_ident = m_allocator.alloc<NodeTermIdent>();
                node_ident->ident = consume();
                auto term = m_allocator.alloc<NodeTerm>();
                term->var = node_ident;
                return term;
            }
        }
        else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr() {
        if (auto term = parse_term()) {
            if (peek().has_value() && (peek().value().Type == TokenType::plus || peek().value().Type == TokenType::mult )) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                if (peek().value().Type == TokenType::plus) {
                    auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                    auto lhs_expr = m_allocator.alloc<NodeExpr>();
                    lhs_expr->var = term.value();
                    bin_expr_add->lhs = lhs_expr;
                    consume();
                    if (auto rhs = parse_term()) {
                        auto rhs_expr = m_allocator.alloc<NodeExpr>();
                        rhs_expr->var = rhs.value();
                        bin_expr_add->rhs = rhs_expr;
                        bin_expr->var = bin_expr_add;
                        auto expr = m_allocator.alloc<NodeExpr>();
                        expr->var = bin_expr;
                        return expr;
                    }
                    else {
                        std::cout << "Bruh! Expected expr";
                    }
                }
                else if (peek().has_value() && peek().value().Type == TokenType::mult) {
                    auto bin_expr_mult = m_allocator.alloc<NodeBinExprMult>();
                    auto lhs_expr = m_allocator.alloc<NodeExpr>();
                    lhs_expr->var = term.value();
                    bin_expr_mult->lhs = lhs_expr;
                    consume();
                    if (auto rhs = parse_term()) {
                        auto rhs_expr = m_allocator.alloc<NodeExpr>();
                        rhs_expr->var = rhs.value();
                        bin_expr_mult->rhs = rhs_expr;
                        bin_expr->var = bin_expr_mult;
                        auto expr = m_allocator.alloc<NodeExpr>();
                        expr->var = bin_expr;
                        return expr;
                    }
                    else {
                        std::cout << "Bruh! Expected expr";
                    }
                }
            }
            else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr-> var = term.value();
                return expr;
            }
        }
        else {
            return {};
        }

        return {};

    }

    std::optional<NodeStmt*> parse_stmt() {
        if (peek().has_value() && peek().value().Type == TokenType::exit) {
            if (peek().value().Type == TokenType::exit) {
                consume();
                if (peek().value().Type == TokenType::open_parent) {
                    consume();
                    if (auto node_expr = parse_expr()) {
                        if (peek().has_value() && peek().value().Type == TokenType::close_parent) {
                            consume();
                            if (peek().has_value() || peek().value().Type != TokenType::semi) {
                                consume();
                                auto node_stmt = m_allocator.alloc<NodeStmtExit>();
                                node_stmt->expr = node_expr.value();
                                auto stmt_node = m_allocator.alloc<NodeStmt>();
                                stmt_node->var = node_stmt;
                                return stmt_node;

                            }
                            else {
                                std::cout << "Bruh! No semi after exit T_T ";
                                exit(EXIT_FAILURE);
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
                            auto node_stmt = m_allocator.alloc<NodeStmtInt>();
                            node_stmt->expr = node_expr.value();
                            node_stmt->ident = ident;
                            auto stmt_node = m_allocator.alloc<NodeStmt>();
                            stmt_node->var = node_stmt;
                            return stmt_node;
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

    }

    std::optional<NodeProg*> parse_prog() {
        std::vector<NodeStmt*>stmts;
        while (peek().has_value()) {
            if (auto node_stmt = parse_stmt()) {
                stmts.push_back(node_stmt.value());
            }
            else {
                std::cout << "Bruh! Invalid stmt T_T ";
                exit(EXIT_FAILURE);
            }
        }
        auto node_prog = m_allocator.alloc<NodeProg>();
        node_prog->stmts = stmts;
        return node_prog;
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
    ArenaAllocator m_allocator;
};

