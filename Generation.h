#pragma once
#include <unordered_map>


class Generator {
public:
    inline explicit Generator(Node::NodeProg prog)
        :m_prog(std::move(prog))
    {

    }

    void gen_expr(const Node::NodeExpr& expr) {
        struct ExprVisitor {
            Generator* gen;
            void operator()(const Node::NodeExprIdent& expr_ident) const {
                if (gen->m_vars.find(expr_ident.ident.Value.value()) == gen->m_vars.end()) {
                    std::cout << "Ident not declared: " << expr_ident.ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                const Var var = gen->m_vars.at(expr_ident.ident.Value.value());
                gen->push("QWORD [rsp + " + std::to_string((gen->m_stack_size - var.stack_loc - 1)*8) + "]\n");

            }
            void operator()(const Node::NodeExprIntLit& expr_int_lit) const{
                gen->m_output << "\tmov rax, " << expr_int_lit.int_lit.Value.value() << "\n";
                gen->push("rax");
            }
        };

        ExprVisitor visitor {.gen = this};
        std::visit(visitor, expr.expr);
    }



    void gen_stmt(const Node::NodeStmt& stmt) {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const Node::NodeStmtExit& stmt_exit) const {
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "\tmov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "\tsyscall\n";
            }
            void operator()(const Node::NodeStmtInt& stmt_int) const {
                if (gen->m_vars.find(stmt_int.ident.Value.value()) != gen->m_vars.end()) {
                    std::cout << "Ident already used: " << stmt_int.ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({stmt_int.ident.Value.value(), Var{.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_int.expr);

            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::stringstream gen_prog() {
        this->m_output << "global _start" << "\n";
        this->m_output << "_start:" << "\n";
        for (const Node::NodeStmt& stmt : m_prog.stmts) {
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
    const Node::NodeProg m_prog;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};
};
