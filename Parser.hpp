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

struct NodeIntExpr;

struct NodeTermParent {
    NodeIntExpr* expr;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParent*> var ;
};

struct NodeBoolTermLit {
    Token bool_lit;
};

struct NodeBoolTerm {
  std::variant<NodeBoolTermLit*, NodeTermIdent*> var;
};

struct NodeBoolExpr {
    NodeBoolTerm* var;
};

struct NodeBinExprAdd {
    NodeIntExpr* lhs;
    NodeIntExpr* rhs;
};

struct NodeBinExprSub {
    NodeIntExpr* lhs;
    NodeIntExpr* rhs;
};

struct NodeBinExprMult {
    NodeIntExpr* lhs;
    NodeIntExpr* rhs;

};

struct NodeBinExprDiv {
    NodeIntExpr* lhs;
    NodeIntExpr* rhs;

};
struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMult*, NodeBinExprDiv*> var;
};

struct NodeIntExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeIntExpr* expr;
};
struct NodeStmtInt {
    Token ident;
    NodeIntExpr* expr;
};

struct NodeStmtBool {
    Token ident;
    NodeBoolExpr* expr;
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeElseIf {
    NodeBoolExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> if_pred;
};

struct NodeElse {
    NodeScope* scope;
};

struct NodeIfPred {
    std::variant<NodeElseIf*, NodeElse*> var;
};

struct NodeStmtIf {
    NodeBoolExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> if_pred;
};

struct NodeStmtVarReassignment {
    Token ident;
    std::variant<NodeIntExpr*, NodeBoolExpr*> expr;
};


struct NodeVarInc {
    Token ident;
};
struct NodeVarDec {
    Token ident;
};

struct NodeInstAdd {
    Token ident;
    NodeIntExpr* expr;
};
struct NodeInstSub {
    Token ident;
    NodeIntExpr* expr;
};
struct NodeInstMul {
    Token ident;
    NodeIntExpr* expr;
};
struct NodeInstDiv {
    Token ident;
    NodeIntExpr* expr;
};

struct NodeStmtInstOp {
    std::variant<NodeInstAdd*, NodeInstSub*, NodeInstMul*, NodeInstDiv*> var;
};

struct NodeStmtRepeat {
    NodeIntExpr* expr;
    NodeScope* scope;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtInt*, NodeStmtBool*, NodeScope*, NodeStmtIf*, NodeStmtVarReassignment*, NodeVarInc*, NodeVarDec*, NodeStmtInstOp*, NodeStmtRepeat*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};


class Parser {

public:
     explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4){

    }

    std::optional<NodeTerm*> parse_int_term() {
        if (peek().has_value() && peek().value().Type == TokenType::int_lit) {
            const auto node_expr_int_lit = m_allocator.alloc<NodeTermIntLit>();
            node_expr_int_lit->int_lit = consume();
            const auto term = m_allocator.alloc<NodeTerm>();
            term->var = node_expr_int_lit;
            return term;
        }
        if (peek().has_value() && peek().value().Type == TokenType::ident) {
            const auto node_ident = m_allocator.alloc<NodeTermIdent>();
            node_ident->ident = consume();
            const auto term = m_allocator.alloc<NodeTerm>();
            term->var = node_ident;
            return term;
        }
         if (peek().has_value() && peek().value().Type == TokenType::open_parent) {
             consume();
             const auto expr = parse_int_expr();
             if (!expr.has_value()) {
                 std::cout << "Bruh! Invalid expr T_T ";
                 exit(EXIT_FAILURE);
             }
             try_consume(TokenType::close_parent, "Bruh! Parent doesnt close!");
             const auto term = m_allocator.alloc<NodeTerm>();
             const auto parent_term = m_allocator.alloc<NodeTermParent>();
             parent_term->expr = expr.value();
             term->var = parent_term;
             return term;
         }
        return {};
    }

    std::optional<NodeIntExpr*> parse_int_expr(const int min_prec = 0) {
         std::optional<NodeTerm*> term_lhs = parse_int_term();
         if (!term_lhs.has_value()) {
             return {};
         }

         auto expr_lhs = m_allocator.alloc<NodeIntExpr>();
         expr_lhs->var = term_lhs.value();

         while (true) {
             const std::optional<Token> curr_token = peek();
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
             const Token op = consume();
             const int next_min_prec = prec.value() + 1;
             const auto expr_rhs = parse_int_expr(next_min_prec);
             if (!expr_rhs.has_value()) {
                 std::cout << "Bruh! Invalid expr T_T ";
                 exit(EXIT_FAILURE);
             }

             const auto expr = m_allocator.alloc<NodeBinExpr>();
             const auto expr_lhs2 = m_allocator.alloc<NodeIntExpr>();
             if (op.Type == TokenType::plus) {
                 const auto add = m_allocator.alloc<NodeBinExprAdd>();
                 expr_lhs2->var = expr_lhs->var;
                 add->lhs = expr_lhs2;
                 add->rhs = expr_rhs.value();
                 expr->var = add;
             } else if (op.Type == TokenType::minus) {
                 const auto sub = m_allocator.alloc<NodeBinExprSub>();
                 expr_lhs2->var = expr_lhs->var;
                 sub->lhs = expr_lhs2;
                 sub->rhs = expr_rhs.value();
                 expr->var = sub;
             }else if (op.Type == TokenType::star) {
                 const auto mult = m_allocator.alloc<NodeBinExprMult>();
                 expr_lhs2->var = expr_lhs->var;
                 mult->lhs = expr_lhs2;
                 mult->rhs = expr_rhs.value();
                 expr->var = mult;
             }else if (op.Type == TokenType::fslash) {
                 const auto div = m_allocator.alloc<NodeBinExprDiv>();
                 expr_lhs2->var = expr_lhs->var;
                 div->lhs = expr_lhs2;
                 div->rhs = expr_rhs.value();
                 expr->var = div;
             }

             expr_lhs->var = expr;

         }
         return expr_lhs;
    }

    std::optional<NodeBoolTerm*> parse_bool_term() {
         if (peek().has_value() && peek().value().Type == TokenType::bool_lit) {
             const auto term = m_allocator.alloc<NodeBoolTerm>();
             const auto term_lit = m_allocator.alloc<NodeBoolTermLit>();
             term_lit->bool_lit = consume();
             term->var = term_lit;
             return term;
         }
         if (peek().has_value() && peek().value().Type == TokenType::ident) {
             const auto node_ident = m_allocator.alloc<NodeTermIdent>();
             node_ident->ident = consume();
             const auto term = m_allocator.alloc<NodeBoolTerm>();
             term->var = node_ident;
             return term;
         }
         return {};
     }

    std::optional<NodeBoolExpr*> parse_bool_expr() {
         if (const auto term = parse_bool_term()) {
             const auto expr = m_allocator.alloc<NodeBoolExpr>();
             expr->var = term.value();
             return expr;
         }

         return {};
     }

    std::optional<NodeScope*> parse_scope() {
         if (peek().has_value() && peek().value().Type == TokenType::open_curly) {
             consume();
             const auto stmt_scope = m_allocator.alloc<NodeScope>();
             while (auto node_stmt = parse_stmt()) {
                 stmt_scope->stmts.push_back(node_stmt.value());
             }
             try_consume(TokenType::close_curly, "Bruh! No close curly.");
             return stmt_scope;
         }
         return {};
     }

    std::optional<NodeIfPred*> parse_if_pred() {
         if (peek().has_value() && peek().value().Type == TokenType::_else) {
             consume();
             if (peek().has_value() && peek().value().Type == TokenType::_if) {
                 consume();
                 try_consume(TokenType::open_parent, "No '(' after if");
                 const auto if_pred = m_allocator.alloc<NodeIfPred>();
                 const auto elseif = m_allocator.alloc<NodeElseIf>();
                 if (const auto expr = parse_bool_expr()) {
                     elseif->expr = expr.value();
                     try_consume(TokenType::close_parent, "No ')' after if");
                     if (const auto scope = parse_scope()) {
                         elseif->scope = scope.value();
                         if (const auto next_if_pred = parse_if_pred()) {
                             elseif->if_pred = next_if_pred.value();
                         }
                         if_pred->var = elseif;
                         return if_pred;
                     }
                     std::cout << " Scope dont parsed" << std::endl;
                     exit(EXIT_FAILURE);
                 }
                 std::cout << " Scope dont parsed" << std::endl;
                 exit(EXIT_FAILURE);
             }
             if (const auto scope = parse_scope()) {
                 const auto node_else = m_allocator.alloc<NodeElse>();
                 node_else->scope = scope.value();
                 const auto node_pred = m_allocator.alloc<NodeIfPred>();
                 node_pred->var = node_else;
                 return node_pred;
             }
             std::cout << " Scope dont parsed" << std::endl;
             exit(EXIT_FAILURE);
         }

         return {};
     }

    std::optional<NodeStmt*> parse_stmt() {
         if (peek().has_value() && peek().value().Type == TokenType::exit) {
            consume();
            try_consume(TokenType::open_parent, "Bruh! No open parenthesis.");
            if (const auto node_expr = parse_int_expr()) {
                try_consume(TokenType::close_parent, "Bruh! No open parenthesis.");
                try_consume(TokenType::semi, "Bruh! No semi after exit T_T ");
                const auto node_stmt = m_allocator.alloc<NodeStmtExit>();
                node_stmt->expr = node_expr.value();
                const auto stmt_node = m_allocator.alloc<NodeStmt>();
                stmt_node->var = node_stmt;
                return stmt_node;
            }
            std::cout << "Bruh! Invalid expr after exit.";
            exit(EXIT_FAILURE);

        }
         if (peek().has_value() && peek().value().Type == TokenType::_int) {
            consume();
            const Token ident = try_consume(TokenType::ident, "Bruh! Invalid or no ident.");
            try_consume(TokenType::eq, "Bruh! No = after ident T_T ");
            if (const auto node_expr = parse_int_expr()) {
                try_consume(TokenType::semi, "Bruh! No semi.");
                const auto node_stmt = m_allocator.alloc<NodeStmtInt>();
                node_stmt->expr = node_expr.value();
                node_stmt->ident = ident;
                const auto stmt_node = m_allocator.alloc<NodeStmt>();
                stmt_node->var = node_stmt;
                return stmt_node;
            }
            std::cout << "Bruh! No value after = T_T ";
            exit(EXIT_FAILURE);
        }
         if (peek().has_value() && peek().value().Type == TokenType::_if) {
             consume();
             try_consume(TokenType::open_parent, "Bruh! No open parenthesis.");
             const auto stmt_if = m_allocator.alloc<NodeStmtIf>();
             if (const auto node_expr = parse_bool_expr()) {
                 stmt_if->expr = node_expr.value();
                 try_consume(TokenType::close_parent, "Bruh! No close parenthesis");
                 if (const auto scope = parse_scope()) {
                     stmt_if->scope = scope.value();
                 }
                 if (const auto if_pred = parse_if_pred()) {
                     stmt_if->if_pred = if_pred.value();
                 }
                 const auto stmt = m_allocator.alloc<NodeStmt>();
                 stmt->var = stmt_if;
                 return stmt;
             }
             std::cout << "Bruh! Invalid expr after if.";
             exit(EXIT_FAILURE);
         }
         if (const auto scope = parse_scope()) {
             const auto stmt = m_allocator.alloc<NodeStmt>();
             stmt->var = scope.value();
             return stmt;
         }
         if (peek().has_value() && peek().value().Type == TokenType::ident) {
             const auto stmt = m_allocator.alloc<NodeStmt>();
             if (peek(1).has_value() && peek(1).value().Type == TokenType::eq){
                 const auto node_var_reassign = m_allocator.alloc<NodeStmtVarReassignment>();
                 node_var_reassign->ident = consume();
                 consume();
                 if (const auto node_expr = parse_int_expr()) {
                     try_consume(TokenType::semi, "Bruh! No semi.");
                     node_var_reassign->expr = node_expr.value();
                     stmt->var = node_var_reassign;
                     return stmt;
                 }
                 std::cout << "Bruh! Invalid expr in reassignment.";
                 exit(EXIT_FAILURE);
             }
             if (peek(1).has_value() && peek(1).value().Type == TokenType::plus && peek(2).has_value() && peek(2).value().Type == TokenType::plus) {
                 const auto node_var_inc = m_allocator.alloc<NodeVarInc>();
                 node_var_inc->ident = consume();
                 consume();
                 consume();
                 try_consume(TokenType::semi, "Bruh! No semi.");
                 stmt->var = node_var_inc;
                 return stmt;
             }
             if (peek(1).has_value() && peek(1).value().Type == TokenType::minus && peek(2).has_value() && peek(2).value().Type == TokenType::minus) {
                 const auto node_var_dec = m_allocator.alloc<NodeVarDec>();
                 node_var_dec->ident = consume();
                 consume();
                 consume();
                 try_consume(TokenType::semi, "Bruh! No semi.");
                 stmt->var = node_var_dec;
                 return stmt;
             }
             if (peek(1).has_value() && peek(1).value().Type == TokenType::plus && peek(2).has_value() && peek(2).value().Type == TokenType::eq) {
                 const auto node_instOp = m_allocator.alloc<NodeStmtInstOp>();
                 const auto node_instAdd = m_allocator.alloc<NodeInstAdd>();
                 node_instAdd->ident = consume();
                 consume();
                 consume();
                 if (const auto node_expr = parse_int_expr()) {
                     node_instAdd->expr = node_expr.value();
                     try_consume(TokenType::semi, "Bruh! No semi.");
                     node_instOp->var = node_instAdd;
                     stmt->var = node_instOp;
                     return stmt;
                 }
             }
             if (peek(1).has_value() && peek(1).value().Type == TokenType::minus && peek(2).has_value() && peek(2).value().Type == TokenType::eq) {
                 const auto node_instOp = m_allocator.alloc<NodeStmtInstOp>();
                 const auto node_instSub = m_allocator.alloc<NodeInstSub>();
                 node_instSub->ident = consume();
                 consume();
                 consume();
                 if (const auto node_expr = parse_int_expr()) {
                     node_instSub->expr = node_expr.value();
                     try_consume(TokenType::semi, "Bruh! No semi.");
                     node_instOp->var = node_instSub;
                     stmt->var = node_instOp;
                     return stmt;
                 }
             }
             if (peek(1).has_value() && peek(1).value().Type == TokenType::star && peek(2).has_value() && peek(2).value().Type == TokenType::eq) {
                 const auto node_instOp = m_allocator.alloc<NodeStmtInstOp>();
                 const auto node_instMul = m_allocator.alloc<NodeInstMul>();
                 node_instMul->ident = consume();
                 consume();
                 consume();
                 if (const auto node_expr = parse_int_expr()) {
                     node_instMul->expr = node_expr.value();
                     try_consume(TokenType::semi, "Bruh! No semi.");
                     node_instOp->var = node_instMul;
                     stmt->var = node_instOp;
                     return stmt;
                 }
             }
             if (peek(1).has_value() && peek(1).value().Type == TokenType::fslash && peek(2).has_value() && peek(2).value().Type == TokenType::eq) {
                 const auto node_instOp = m_allocator.alloc<NodeStmtInstOp>();
                 const auto node_instDiv = m_allocator.alloc<NodeInstDiv>();
                 node_instDiv->ident = consume();
                 consume();
                 consume();
                 if (const auto node_expr = parse_int_expr()) {
                     node_instDiv->expr = node_expr.value();
                     try_consume(TokenType::semi, "Bruh! No semi.");
                     node_instOp->var = node_instDiv;
                     stmt->var = node_instOp;
                     return stmt;
                 }
             }
         }
         if (peek().has_value() && peek().value().Type == TokenType::repeat) {
             consume();
             try_consume(TokenType::open_parent, "Bruh! No '('");
             if (const auto node_expr = parse_int_expr()) {
                 try_consume(TokenType::close_parent, "Bruh! No ')'");
                 if (const auto scope = parse_scope()) {
                     const auto stmt = m_allocator.alloc<NodeStmt>();
                     const auto node_repeat = m_allocator.alloc<NodeStmtRepeat>();
                     node_repeat->expr = node_expr.value();
                     node_repeat->scope = scope.value();
                     stmt->var = node_repeat;
                     return stmt;
                 }
             }
             std::cout << "Bruh! invalid expr! \n";
             exit(EXIT_FAILURE);
         }
         if (peek().has_value() && peek().value().Type == TokenType::_bool) {
             consume();
             const Token ident = try_consume(TokenType::ident, "Bruh! Invalid or no ident.");
             try_consume(TokenType::eq, "Bruh! No = after ident T_T ");
             if (const auto node_bool_expr = parse_bool_expr()) {
                 try_consume(TokenType::semi, "Bruh! No semi.");
                 const auto node_bool = m_allocator.alloc<NodeStmtBool>();
                 node_bool->expr = node_bool_expr.value();
                 node_bool->ident = ident;
                 const auto stmt = m_allocator.alloc<NodeStmt>();
                 stmt->var = node_bool;
                 return stmt;
             }
             std::cout << "Bruh! invalid expr! \n";
             exit(EXIT_FAILURE);
         }

        return {};

    }

    std::optional<NodeProg*> parse_prog() {
        std::vector<NodeStmt*>stmts;
        while (peek().has_value()) {
            if (const auto node_stmt = parse_stmt()) {
                stmts.push_back(node_stmt.value());
            }
            else {
                std::cout << "Bruh! Invalid stmt T_T ";
                exit(EXIT_FAILURE);
            }
        }
        const auto node_prog = m_allocator.alloc<NodeProg>();
        node_prog->stmts = stmts;
        return node_prog;
    }



private:
    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens[m_index + offset];

    }

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

