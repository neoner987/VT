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

    std::optional<NodeExpr*> parse_expr(int min_prec = 0) {
         std::optional<NodeTerm*> term_lhs = parse_term();
         if (!term_lhs.has_value()) {
             return {};
         }

         auto expr_lhs = m_allocator.alloc<NodeExpr>();
         expr_lhs->var = term_lhs.value();

         while (true) {
             std::optional<Token> curr_token = peek();
             std::optional<int> prec;
             if (curr_token.has_value()) {
                 prec = bin_prec(curr_token->Type);
                 if (!prec.has_value() || prec < min_prec) {
                     break;
                 }

             }
             else {
                 break;
             }
             Token op = consume();
             int next_min_prec = prec.value() + 1;
             auto expr_rhs = parse_expr(next_min_prec);
             if (!expr_rhs.has_value()) {
                 std::cout << "Bruh! Invalid expr T_T ";
                 exit(EXIT_FAILURE);
             }

             auto expr = m_allocator.alloc<NodeBinExpr>();
             auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
             if (op.Type == TokenType::plus) {
                 auto add = m_allocator.alloc<NodeBinExprAdd>();
                 expr_lhs2->var = expr_lhs->var;
                 add->lhs = expr_lhs2;
                 add->rhs = expr_rhs.value();
                 expr->var = add;
             } else if (op.Type == TokenType::minus) {
                 auto sub = m_allocator.alloc<NodeBinExprSub>();
                 expr_lhs2->var = expr_lhs->var;
                 sub->lhs = expr_lhs2;
                 sub->rhs = expr_rhs.value();
                 expr->var = sub;
             }else if (op.Type == TokenType::mult) {
                 auto mult = m_allocator.alloc<NodeBinExprMult>();
                 expr_lhs2->var = expr_lhs->var;
                 mult->lhs = expr_lhs2;
                 mult->rhs = expr_rhs.value();
                 expr->var = mult;
             }

             expr_lhs->var = expr;

         }
         return expr_lhs;
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

