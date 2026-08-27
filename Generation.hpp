#pragma once
#include <algorithm>
#include <map>
#include "Parser.hpp"


class Generator {
public:
    explicit Generator(const NodeProg* prog)
        :m_prog(prog){}

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
          Generator& gen;
            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen.m_output << "\tmov rax, " << term_int_lit->int_lit.Value.value() << "\n";
                gen.push("rax");
            }
            void operator()(const NodeTermIdent* term_ident) const {
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(), [&](const Var& var){return var.ident  == term_ident->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << term_ident->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.push("QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "]");
            }
            void operator()(const NodeTermParent* term_parent) const {
                gen.gen_expr(term_parent->expr);
            }
        };

        TermVisitor visitor {.gen = *this};
        std::visit(visitor, term->var);

    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator& gen;
            void operator()(const NodeBinExprAdd* add_expr) const {
                gen.gen_expr(add_expr->rhs);
                gen.gen_expr(add_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "\tadd rax, rbx" << "\n";
                gen.push("rax");
            }
            void operator()(const NodeBinExprSub* sub_expr) const {
                gen.gen_expr(sub_expr->rhs);
                gen.gen_expr(sub_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "\tsub rax, rbx" << "\n";
                gen.push("rax");
            }
            void operator()(const NodeBinExprMult* mult_expr) const {
                gen.gen_expr(mult_expr->rhs);
                gen.gen_expr(mult_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "\tmul rbx" << "\n";
                gen.push("rax");
            }
            void operator()(const NodeBinExprDiv* div_expr) const {
                gen.gen_expr(div_expr->rhs);
                gen.gen_expr(div_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "\tdiv rbx" << "\n";
                gen.push("rax");
            }
        };

        BinExprVisitor visitor {.gen = *this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {
            Generator& gen;
            void operator()(const NodeTerm* expt_term) const {
                gen.gen_term(expt_term);
            }
            void operator()(const NodeBinExpr* bin_expt) const{
                gen.gen_bin_expr(bin_expt);
            }
        };

        ExprVisitor visitor {.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_scope(const NodeScope* scope) {
        begin_scope();
        for (const NodeStmt* stmt : scope->stmts) {
            gen_stmt(stmt);
        }
        end_scope();
    }

    void gen_if_pred(const NodeIfPred* if_pred, const std::string& endStmt) {

        struct IfPredVisitor {
            Generator& gen;
            const std::string& endStmt;
            void operator()(const NodeElseIf* nodeElseIf) const {
                gen.gen_expr(nodeElseIf->expr);
                gen.pop("rax");
                const std::string endIf = gen.create_label();

                gen.m_output << "\ttest rax, rax" << "\n";
                gen.m_output << "\tjnz " + endIf + "\n";
                gen.gen_scope(nodeElseIf->scope);
                if (nodeElseIf->if_pred.has_value()) {
                    gen.m_output << "\tjmp " << endStmt << "\n";
                    gen.m_output << "\t" << endIf << ":\n";
                    gen.gen_if_pred(nodeElseIf->if_pred.value(), endStmt);
                }
                else {
                    gen.m_output << "\t" << endIf << ":\n";
                }
            }

            void operator() (const NodeElse* nodeElse) const {
                gen.gen_scope(nodeElse->scope);
            }
        };

        IfPredVisitor visitor{.gen = *this, .endStmt = endStmt};
        std::visit(visitor, if_pred->var);
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator& gen;
            void operator()(const NodeStmtExit* stmt_exit) const {
                gen.gen_expr(stmt_exit->expr);
                gen.m_output << "\tmov rax, 60\n";
                gen.pop("rdi");
                gen.m_output << "\tsyscall\n";
            }
            void operator()(const NodeStmtInt* stmt_int) const {
                if (std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var)
                    {return var.ident  == stmt_int->ident.Value.value();})
                    != gen.m_vars.cend()) {
                    std::cout << "Ident already used: " << stmt_int->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_vars.push_back(Var{.ident = stmt_int->ident.Value.value(), .stack_loc = gen.m_stack_size });
                gen.gen_expr(stmt_int->expr);
            }
            void operator()(const NodeScope* scope) const {
                gen.gen_scope(scope);
            }
            void operator()(const NodeStmtIf* stmt_if) const {
                gen.gen_expr(stmt_if->expr);
                gen.pop("rax");
                const std::string endIf = gen.create_label();

                gen.m_output << "\ttest rax, rax" << "\n";
                gen.m_output << "\tjnz " + endIf + "\n";
                gen.gen_scope(stmt_if->scope);

                if (stmt_if->if_pred.has_value()) {
                    const std::string endStmt = gen.create_label();
                    gen.m_output << "\tjmp " << endStmt << "\n";
                    gen.m_output << "\t" << endIf << ":\n";
                    gen.gen_if_pred(stmt_if->if_pred.value(), endStmt);
                    gen.m_output << "\t"<< endStmt << ":\n";
                }
                else {
                    gen.m_output << "\t" << endIf << ":\n";
                }
            }
            void operator() (const NodeStmtVarReassignment* stmt_var_reassignment) const {
                gen.gen_expr(stmt_var_reassignment->expr);
                gen.pop("rax");
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == stmt_var_reassignment->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << stmt_var_reassignment->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_output << "mov QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "], rax\n";
            }
        };

        StmtVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::stringstream gen_prog() {
        this->m_output << "global _start" << "\n";
        this->m_output << "_start:" << "\n";
        for (const NodeStmt* stmt : m_prog->stmts) {
            gen_stmt(stmt);
        }
        this->m_output << "\tmov rax, 60" << "\n";
        this->m_output << "\t" << "mov rdi, 0\n";
        this->m_output << "\tsyscall" << "\n";

        return std::move(this->m_output);
    }


private:

    void begin_scope() {
        m_scopes.push_back(m_vars.size());
    }
    void end_scope() {
        const size_t pop_count = m_vars.size() - m_scopes.back();
        m_output << "\tadd rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count;
        for (int a = 0; a < pop_count; a++) {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }
    void push(const std::string& reg) {
        this->m_output << "\tpush " << reg << "\n";
        m_stack_size++;
    }
    void pop (const std::string& reg) {
        this->m_output << "\tpop " << reg << "\n";
        m_stack_size--;
    }
    std::string create_label() {
        return "Label" + std::to_string(label_count++);
    }

    struct Var {
        std::string ident;
        size_t stack_loc;

    };

    int label_count = 0;
    std::stringstream m_output;
    const NodeProg* m_prog;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars {};
    std::vector< size_t > m_scopes {};
};
