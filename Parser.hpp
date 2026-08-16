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


struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMult {
    NodeExpr* lhs;
    NodeExpr* rhs;

};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMult*> var;
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
     explicit Parser(std::vector<Token> tokens)
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
        if (peek().has_value() && peek().value().Type == TokenType::ident) {
            if (peek().value().Type == TokenType::ident) {
                auto node_ident = m_allocator.alloc<NodeTermIdent>();
                node_ident->ident = consume();
                auto term = m_allocator.alloc<NodeTerm>();
                term->var = node_ident;
                return term;
            }
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr() {
        if (auto term = parse_term()) {
            if (peek().has_value() && (peek().value().Type == TokenType::plus || peek().value().Type == TokenType::mult
                || peek().value().Type == TokenType::minus )) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                if (peek().value().Type == TokenType::plus) {
                    auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                    auto lhs_expr = m_allocator.alloc<NodeExpr>();
                    lhs_expr->var = term.value();
                    bin_expr_add->lhs = lhs_expr;
                    consume();
                    if (auto rhs = parse_expr()) {
                        bin_expr_add->rhs = rhs.value();;
                        bin_expr->var = bin_expr_add;
                        auto expr = m_allocator.alloc<NodeExpr>();
                        expr->var = bin_expr;
                        return expr;
                    }
                    std::cout << "Bruh! Expected expr";
                    exit(EXIT_FAILURE);
                }
                if (peek().value().Type == TokenType::minus) {
                    auto bin_expr_sub = m_allocator.alloc<NodeBinExprSub>();
                    auto lhs_expr = m_allocator.alloc<NodeExpr>();
                    lhs_expr->var = term.value();
                    bin_expr_sub->lhs = lhs_expr;
                    consume();
                    if (auto rhs = parse_expr()) {
                        bin_expr_sub->rhs = rhs.value();;
                        bin_expr->var = bin_expr_sub;
                        auto expr = m_allocator.alloc<NodeExpr>();
                        expr->var = bin_expr;
                        return expr;
                    }
                    std::cout << "Bruh! Expected expr";
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().Type == TokenType::mult) {
                    auto bin_expr_mult = m_allocator.alloc<NodeBinExprMult>();
                    auto lhs_expr = m_allocator.alloc<NodeExpr>();
                    lhs_expr->var = term.value();
                    bin_expr_mult->lhs = lhs_expr;
                    consume();
                    if (auto rhs = parse_expr()) {
                        bin_expr_mult->rhs = rhs.value();
                        bin_expr->var = bin_expr_mult;
                        auto expr = m_allocator.alloc<NodeExpr>();
                        expr->var = bin_expr;
                        return expr;
                    }
                    std::cout << "Bruh! Expected expr";
                    exit(EXIT_FAILURE);
                }
            }
            else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr-> var = term.value();
                return expr;
            }
        }
        return {};

    }

    std::optional<NodeStmt*> parse_stmt() {
        if (peek().has_value() && peek().value().Type == TokenType::exit) {
            consume();
            try_consume(TokenType::open_parent, "Bruh! No open parenthesis.");
            if (auto node_expr = parse_expr()) {
                try_consume(TokenType::close_parent, "Bruh! No open parenthesis.");
                try_consume(TokenType::semi, "Bruh! No semi after exit T_T ");
                auto node_stmt = m_allocator.alloc<NodeStmtExit>();
                node_stmt->expr = node_expr.value();
                auto stmt_node = m_allocator.alloc<NodeStmt>();
                stmt_node->var = node_stmt;
                return stmt_node;
            }
            std::cout << "Bruh! Invalid expr after exit.";
            exit(EXIT_FAILURE);

        }
        if (peek().has_value() && peek().value().Type == TokenType::_int) {
            consume();
            Token ident = try_consume(TokenType::ident, "Bruh! Invalid or no ident.");
            try_consume(TokenType::eq, "Bruh! No = after ident T_T ");
            if (auto node_expr = parse_expr()) {
                try_consume(TokenType::semi, "Bruh! No semi.");
                auto node_stmt = m_allocator.alloc<NodeStmtInt>();
                node_stmt->expr = node_expr.value();
                node_stmt->ident = ident;
                auto stmt_node = m_allocator.alloc<NodeStmt>();
                stmt_node->var = node_stmt;
                return stmt_node;
            }
            std::cout << "Bruh! No value after = T_T ";
            exit(EXIT_FAILURE);
        }
        std::cout << "Bruh! Invalid stmt";
        exit(EXIT_FAILURE);

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
    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens[m_index + offset];

    };

    Token consume(){
        return m_tokens[m_index++];
    }

    Token try_consume(const TokenType type, const std::string& msg) {
        if (peek().has_value() && peek().value().Type == type) {
            return consume();
        }
        std::cout << msg << std::endl;
        exit(EXIT_FAILURE);
    }
    size_t m_index = 0;
    const std::vector<Token> m_tokens;
    ArenaAllocator m_allocator;
};

