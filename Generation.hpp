#pragma once
#include <unordered_map>
#include "Parser.hpp"


class Generator {
public:
    inline explicit Generator(NodeProg* prog)
        :m_prog(std::move(prog))
    {

    }

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
          Generator* gen;
            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen->m_output << "\tmov rax, " << term_int_lit->int_lit.Value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeTermIdent* term_ident) const {
                if (gen->m_vars.find(term_ident->ident.Value.value()) == gen->m_vars.end()) {
                    std::cout << "Ident not declared: " << term_ident->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                const Var var = gen->m_vars.at(term_ident->ident.Value.value());
                gen->push("QWORD [rsp + " + std::to_string((gen->m_stack_size - var.stack_loc - 1)*8) + "]\n");
            }
        };

        TermVisitor visitor {.gen = this};
        std::visit(visitor, term->var);

    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator* gen;
            void operator()(const NodeBinExprAdd* add_expr) const {
                gen->gen_expr(add_expr->rhs);
                gen->gen_expr(add_expr->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\tadd rax, rbx" << "\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprSub* sub_expr) const {
                gen->gen_expr(sub_expr->rhs);
                gen->gen_expr(sub_expr->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\tsub rax, rbx" << "\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprMult* mult_expr) const {
                gen->gen_expr(mult_expr->rhs);
                gen->gen_expr(mult_expr->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\timul rax, rbx" << "\n";
                gen->push("rax");
            }
        };

        BinExprVisitor visitor {.gen = this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {
            Generator* gen;
            void operator()(const NodeTerm* expt_term) const {
                gen->gen_term(expt_term);
            }
            void operator()(const NodeBinExpr* bin_expt) const{
                gen->gen_bin_expr(bin_expt);
            }
        };

        ExprVisitor visitor {.gen = this};
        std::visit(visitor, expr->var);
    }



    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit* stmt_exit) const {
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "\tmov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "\tsyscall\n";
            }
            void operator()(const NodeStmtInt* stmt_int) const {
                if (gen->m_vars.find(stmt_int->ident.Value.value()) != gen->m_vars.end()) {
                    std::cout << "Ident already used: " << stmt_int->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({stmt_int->ident.Value.value(), Var{.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_int->expr);

            }
        };

        StmtVisitor visitor{.gen = this};
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
    void push(const std::string reg) {
        this->m_output << "\tpush " << reg << "\n";
        m_stack_size++;
    }
    void pop (const std::string reg) {
        this->m_output << "\tpop " << reg << "\n";
        m_stack_size--;
    }

    struct Var {
        size_t stack_loc;

    };

    std::stringstream m_output;
    const NodeProg* m_prog;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};
};
