#pragma once
#include <algorithm>
#include <map>
#include "Parser.hpp"


class Generator {
public:
    explicit Generator(const NodeProg* prog)
        :m_prog(prog){}

    void gen_int_term(const NodeTermInt* term) {
        struct TermVisitor {
          Generator& gen;
            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen.m_start << "\tmov rax, " << term_int_lit->int_lit.Value.value() << "\n";
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
            void operator()(const NodeTermIntParent* term_parent) const {
                gen.gen_int_expr(term_parent->expr);
            }
        };

        TermVisitor visitor {.gen = *this};
        std::visit(visitor, term->var);

    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator& gen;
            void operator()(const NodeBinExprAdd* add_expr) const {
                gen.gen_int_expr(add_expr->rhs);
                gen.gen_int_expr(add_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tadd rax, rbx" << "\n";
                gen.push("rax");
            }
            void operator()(const NodeBinExprSub* sub_expr) const {
                gen.gen_int_expr(sub_expr->rhs);
                gen.gen_int_expr(sub_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tsub rax, rbx" << "\n";
                gen.push("rax");
            }
            void operator()(const NodeBinExprMult* mult_expr) const {
                gen.gen_int_expr(mult_expr->rhs);
                gen.gen_int_expr(mult_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tmul rbx" << "\n";
                gen.push("rax");
            }
            void operator()(const NodeBinExprDiv* div_expr) const {
                gen.gen_int_expr(div_expr->rhs);
                gen.gen_int_expr(div_expr->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tdiv rbx" << "\n";
                gen.push("rax");
            }
        };

        BinExprVisitor visitor {.gen = *this};
        std::visit(visitor, bin_expr->var);
    }
    void gen_int_expr(const NodeIntExpr* expr) {
        struct ExprVisitor {
            Generator& gen;
            void operator()(const NodeTermInt* expt_term) const {
                gen.gen_int_term(expt_term);
            }
            void operator()(const NodeBinExpr* bin_expt) const{
                gen.gen_bin_expr(bin_expt);
            }
        };

        ExprVisitor visitor {.gen = *this};
        std::visit(visitor, expr->var);
    }
    size_t gen_str_term(const NodeTermStr* term, const std::string& set_ident = {}) {
        struct StrTermVisitor {
            Generator& gen;
            const std::string& set_ident;
            size_t operator() (const NodeStr_lit* str_lit) const {
                if (str_lit->text.Value->length() <= 8) {
                    size_t start = gen.m_stack_size;

                    gen.m_start << "\tmov rax, \"" << str_lit->text.Value.value() << "\" \n";
                    gen.push("rax");

                    gen.m_string.push_back(String( {.ident = set_ident, .start_loc = start, .end_loc = gen.m_stack_size} ));
                    return gen.m_stack_size - start;
                }
                std::string buff = str_lit->text.Value.value();
                size_t start = gen.m_stack_size;
                while (buff.length() > 8) {
                    gen.m_start << "\tmov rax, \"" << buff.substr(0, 8) << "\" \n";
                    gen.push("rax");
                    buff = buff.substr(8, buff.length());
                }
                gen.m_start << "\tmov rax, \"" << buff.substr(0, 8) << "\" \n";
                gen.push("rax");
                gen.m_string.push_back(String( {.ident = set_ident, .start_loc = start, .end_loc = gen.m_stack_size} ));
                return gen.m_stack_size - start;
            }
            size_t operator() (const NodeTermIdent* ident) const {
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var)
                    {return var.ident  == ident->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << ident->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                const auto str = std::find_if(gen.m_string.cbegin(),  gen.m_string.cend(),
                    [&](const String& var)
                    {return var.ident  == ident->ident.Value.value();});

                if ( str == gen.m_string.cend()) {
                    std::cout << "Ident not string: " << ident->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                size_t num_loop = (str->end_loc - str->start_loc);
                gen.m_start << "\tmov rcx, " << std::to_string(num_loop + 1) <<"\n";
                gen.m_start << "\tmov rbx, rsp\n";
                std::string label = gen.create_label();
                gen.m_start << "\t"<< label <<":\n";

                gen.push("QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1 ) * 8 ) + " + rcx * 8]");

                gen.m_start << "\tloop " << label << "\n";
                gen.m_start << "\tmov rsp, rbx\n";
                return num_loop;
            }
        };
        StrTermVisitor visitor{.gen = *this, .set_ident =set_ident};
        return std::visit(visitor, term->var);
    }

    void gen_scope(const NodeScope* scope) {
        begin_scope();
        for (const NodeStmt* stmt : scope->stmts) {
            gen_stmt(stmt);
        }
        end_scope();

    }

    void gen_bool_term(const NodeBoolTerm* expr) {
        struct BoolExprVisitor {
            Generator& gen;

            void operator() (const NodeBoolTermLit* term_bool_lit) const {
                if (term_bool_lit->bool_lit.Value.value() == "правда") {
                    gen.m_start << "\tmov al, 11111111b\n";
                    gen.push("rax");
                }
                else {
                    gen.m_start << "\tmov al, 00000000b\n";
                    gen.push("rax");
                }
            }
            void operator() (const NodeTermIdent* term_ident) const {
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(), [&](const Var& var){return var.ident  == term_ident->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << term_ident->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.push("QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "]");
            }
            void operator() (const NodeBoolTermParent* term) const {
                gen.gen_bool_expr(term->expr);
            }
            void operator() (const NodeBoolTermNot* NotTerm) const {
                gen.gen_bool_term(NotTerm->term);
                gen.pop("rax");
                gen.m_start << "\tnot al\n";
                gen.push("rax");
            }
        };

        BoolExprVisitor visitor {.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_bool_op_expr (const NodeBoolExprOp*  opExpr) {
        struct BoolOpExprVisitor {
            Generator& gen;
            void operator() (const NodeBoolExprAnd* AndExpr) const {
                gen.gen_bool_expr(AndExpr->lhs);
                gen.gen_bool_expr(AndExpr->rhs);
                gen.pop("rax");
                gen.pop("r15");
                gen.m_start << "\tand r15b, r15b\n";
                gen.push("rax");
            }
            void operator() (const NodeBoolExprOr* OrExpr) const {
                gen.gen_bool_expr(OrExpr->lhs);
                gen.gen_bool_expr(OrExpr->rhs);
                gen.pop("rax");
                gen.pop("r15");
                gen.m_start << "\tor al, r15b\n";
                gen.push("rax");
            }
            void operator() (const NodeBoolOpG* nodeOpG) const {
                gen.gen_int_expr(nodeOpG->lhs);
                gen.gen_int_expr(nodeOpG->rhs);
                gen.m_start << "\txor rcx, rcx\n";
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tcmp rbx, rax\n";
                gen.m_start << "\tseta cl\n";
                gen.push("RCX");
            }
            void operator() (const NodeBoolOpL* nodeOpL) const {
                gen.gen_int_expr(nodeOpL->lhs);
                gen.gen_int_expr(nodeOpL->rhs);
                gen.m_start << "\txor rcx, rcx\n";
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tcmp rbx, rax\n";
                gen.m_start << "\tsetb cl\n";
                gen.push("RCX");
            }
            void operator() (const NodeBoolOpGE* nodeOpGE) const {
                gen.gen_int_expr(nodeOpGE->lhs);
                gen.gen_int_expr(nodeOpGE->rhs);
                gen.m_start << "\txor rcx, rcx\n";
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tcmp rbx, rax\n";
                gen.m_start << "\tsetae cl\n";
                gen.push("RCX");
            }
            void operator() (const NodeBoolOpLE* nodeOpLE) const {
                gen.gen_int_expr(nodeOpLE->lhs);
                gen.gen_int_expr(nodeOpLE->rhs);
                gen.m_start << "\txor rcx, rcx\n";
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tcmp rbx, rax\n";
                gen.m_start << "\tsetbe cl\n";
                gen.push("RCX");
            }
            void operator() (const NodeBoolOpEq* nodeOpEq) const {
                gen.gen_int_expr(nodeOpEq->lhs);
                gen.gen_int_expr(nodeOpEq->rhs);
                gen.m_start << "\txor rcx, rcx\n";
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tcmp rbx, rax\n";
                gen.m_start << "\tsete cl\n";
                gen.push("RCX");
            }
            void operator() (const NodeBoolOpNE* nodeOpNE) const {
                gen.gen_int_expr(nodeOpNE->lhs);
                gen.gen_int_expr(nodeOpNE->rhs);
                gen.m_start << "\txor rcx, rcx\n";
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_start << "\tcmp rbx, rax\n";
                gen.m_start << "\tsetne cl\n";
                gen.push("RCX");
            }
        };

        BoolOpExprVisitor visitor {.gen = *this};
        std::visit(visitor, opExpr->var);
    }

    void gen_bool_expr(const NodeBoolExpr* expr) {
        struct BoolExprVisitor {
            Generator& gen;
            void operator() (const NodeBoolTerm* term) const {
                gen.gen_bool_term(term);
            }
            void operator() (const NodeBoolExprOp* opExpr) const {
                gen.gen_bool_op_expr(opExpr);
            }
        };

        BoolExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_if_pred(const NodeIfPred* if_pred, const std::string& endStmt) {

        struct IfPredVisitor {
            Generator& gen;
            const std::string& endStmt;
            void operator()(const NodeElseIf* nodeElseIf) const {
                gen.gen_bool_expr(nodeElseIf->expr);
                gen.pop("rax");
                const std::string endIf = gen.create_label();

                gen.m_start << "\ttest al, al" << "\n";
                gen.m_start << "\tjz " + endIf + "\n";
                gen.gen_scope(nodeElseIf->scope);
                if (nodeElseIf->if_pred.has_value()) {
                    gen.m_start << "\tjmp " << endStmt << "\n";
                    gen.m_start << "\t" << endIf << ":\n";
                    gen.gen_if_pred(nodeElseIf->if_pred.value(), endStmt);
                }
                else {
                    gen.m_start << "\t" << endIf << ":\n";
                }
            }

            void operator() (const NodeElse* nodeElse) const {
                gen.gen_scope(nodeElse->scope);
            }
        };

        IfPredVisitor visitor{.gen = *this, .endStmt = endStmt};
        std::visit(visitor, if_pred->var);
    }

    void gen_inst_op(const NodeStmtInstOp* stmt) {

        struct InstOpVisitor {
            Generator& gen;

            void operator() ( const NodeInstAdd* instAdd) const {
                gen.gen_int_expr(instAdd->expr);
                gen.pop("rax");
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == instAdd->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << instAdd->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tadd QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "], rax \n";
            }
            void operator() ( const NodeInstSub* instSub) const {
                gen.gen_int_expr(instSub->expr);
                gen.pop("rax");
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == instSub->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << instSub->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tsub QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "], rax\n";
            }
            void operator() ( const NodeInstMul* instMul) const {
                gen.gen_int_expr(instMul->expr);
                gen.pop("rax");
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == instMul->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << instMul->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tmul QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "]\n";
                gen.m_start << "\tmov QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "], rax\n";
            }
            void operator() ( const NodeInstDiv* instDiv) const {
                gen.gen_int_expr(instDiv->expr);
                gen.pop("rbx");
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == instDiv->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << instDiv->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tmov rax, QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "]\n";
                gen.m_start << "\tdiv rbx\n";
                gen.m_start << "\tmov QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "], rax\n";
            }
        };


        InstOpVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    void gen_reassignment_expr(const NodeStmtVarReassignment* reassignment) {
        struct reassignmentVisitior{
            Generator& gen;

            void operator() (const NodeIntExpr* int_expr) const {
                gen.gen_int_expr(int_expr);
            }

            void operator() (const NodeBoolExpr* bool_expr) const {
                gen.gen_bool_expr(bool_expr);
            }
        };

        reassignmentVisitior visitor{.gen = *this};
        std::visit(visitor, reassignment->expr);
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator& gen;
            void operator()(const NodeStmtExit* stmt_exit) const {
                gen.gen_int_expr(stmt_exit->expr);
                gen.m_start << "\tmov rax, 60\n";
                gen.pop("rdi");
                gen.m_start << "\tsyscall\n";
            }
            void operator()(const NodeStmtInt* stmt_int) const {
                if (std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var)
                    {return var.ident  == stmt_int->ident.Value.value();})
                    != gen.m_vars.cend()) {
                    std::cout << "Ident already used: " << stmt_int->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_vars.push_back(Var{.ident = stmt_int->ident.Value.value(), .stack_loc = gen.m_stack_size});
                gen.gen_int_expr(stmt_int->expr);
            }
            void operator() (const NodeStmtBool* stmt_bool) const {
                if (std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var)
                    {return var.ident  == stmt_bool->ident.Value.value();})
                    != gen.m_vars.cend()) {
                    std::cout << "Ident already used: " << stmt_bool->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                    }
                gen.m_vars.push_back(Var{.ident = stmt_bool->ident.Value.value(), .stack_loc = gen.m_stack_size });
                gen.gen_bool_expr(stmt_bool->expr);
            }
            void operator() (const NodeStmtStr* stmt_str) const {
                if (std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var)
                    {return var.ident  == stmt_str->ident.Value.value();})
                    != gen.m_vars.cend()) {
                    std::cout << "Ident already used: " << stmt_str->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                    }
                gen.m_vars.push_back(Var{.ident = stmt_str->ident.Value.value(), .stack_loc = gen.m_stack_size });
                gen.gen_str_term(stmt_str->term, stmt_str->ident.Value.value());

            }
            void operator()(const NodeScope* scope) const {
                gen.gen_scope(scope);
            }
            void operator()(const NodeStmtIf* stmt_if) const {
                gen.gen_bool_expr(stmt_if->expr);
                gen.pop("rax");
                const std::string endIf = gen.create_label();

                gen.m_start << "\ttest al, al" << "\n";
                gen.m_start << "\tjz " + endIf + "\n";
                gen.gen_scope(stmt_if->scope);

                if (stmt_if->if_pred.has_value()) {
                    const std::string endStmt = gen.create_label();
                    gen.m_start << "\tjmp " << endStmt << "\n";
                    gen.m_start << "\t" << endIf << ":\n";
                    gen.gen_if_pred(stmt_if->if_pred.value(), endStmt);
                    gen.m_start << "\t"<< endStmt << ":\n";
                }
                else {
                    gen.m_start << "\t" << endIf << ":\n";
                }
            }
            void operator() (const NodeStmtVarReassignment* stmt_var_reassignment) const {
                gen.gen_reassignment_expr(stmt_var_reassignment);
                gen.pop("rax");
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == stmt_var_reassignment->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << stmt_var_reassignment->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tmov QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "], rax\n";
            }
            void operator() (const NodeVarInc* stmt_var_reassignment) const {
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == stmt_var_reassignment->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << stmt_var_reassignment->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tinc QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "]\n";
            }
            void operator() (const NodeVarDec* stmt_var_reassignment) const {
                const auto it = std::find_if(gen.m_vars.cbegin(),  gen.m_vars.cend(),
                    [&](const Var& var){return var.ident
                        == stmt_var_reassignment->ident.Value.value();});
                if ( it == gen.m_vars.cend()) {
                    std::cout << "Ident not declared: " << stmt_var_reassignment->ident.Value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_start << "\tdec QWORD [rsp + " + std::to_string((gen.m_stack_size - it->stack_loc - 1)*8) + "]\n";
            }
            void operator() (const NodeStmtInstOp* op_stmt) const {
                gen.gen_inst_op(op_stmt);
            }
            void operator() (const NodeStmtRepeat* node_repeat) const {
                gen.gen_int_expr(node_repeat->expr);
                gen.pop("r15");
                std::string end_label = gen.create_label();
                gen.m_start << "\ttest r15, r15\n";
                gen.m_start << "\tjz " << end_label << "\n";
                gen.m_start << "\tmov r14, rsp\n";
                std::string loop_label = gen.create_label();
                gen.m_start << "\t" << loop_label << ":\n";
                gen.gen_scope(node_repeat->scope);
                gen.m_start << "\tdec r15\n";
                gen.m_start << "\ttest r15, r15\n";
                gen.m_start << "\tjne " << loop_label << "\n";
                gen.m_start << "\tmov rsp, r14\n";
                gen.m_start << "\t" << end_label << ":\n";

            }
            void operator() (const NodeStmtWhile* stmt_while) const {
                const std::string endLoop = gen.create_label();
                const std::string startLoop = gen.create_label();
                gen.m_start << "\t" << startLoop << ":\n";

                gen.gen_bool_expr(stmt_while->expr);
                gen.pop("rax");
                gen.m_start << "\ttest al, al" << "\n";
                gen.m_start << "\tjz " + endLoop + "\n";

                gen.gen_scope(stmt_while->scope);
                gen.m_start << "\tjmp " + startLoop + "\n";

                gen.m_start << "\t" << endLoop << ":\n";

            }
            void operator() (const NodeStmtPrint* node_print) const {
                size_t num_of_loop = gen.gen_str_term(node_print->arg);
                gen.m_start << "\tmov dword [rsp + " << num_of_loop * 8 << "], 0\n";
                gen.m_start << "\tmov rbx, " << num_of_loop << "\n";
                gen.m_start << "\tlea rsi, [rsp + " << (num_of_loop - 1) * 8 <<"] \n";
                std::string label_loop = gen.create_label();
                gen.m_start << "\t" << label_loop << ":\n";
                gen.m_start << "\tcall len_msg" << "\n";
                gen.m_start << "\tcmp rdx, 8" << "\n";
                std::string label_size = gen.create_label();
                gen.m_start << "\tjle " << label_size <<"\n";
                gen.m_start << "\tmov rdx, 8" << "\n";
                gen.m_start << "\t" << label_size << ":\n";
                gen.m_start << "\tmov rax, 1" << "\n";
                gen.m_start << "\tmov rdi, 1" << "\n";
                gen.m_start << "\tsyscall" << "\n";
                gen.m_start << "\tsub rsi, 8\n";
                gen.m_start << "\tdec rbx" << "\n";
                gen.m_start << "\ttest rbx, rbx\n";
                gen.m_start << "\tjnz " << label_loop << "\n";
                if (node_print->bool_lit.has_value()) {
                    if (node_print->bool_lit->Value.value() == "правда") {
                        gen.m_start << "\tmov rax, 1" << "\n";
                        gen.m_start << "\tmov rdi, 1" << "\n";
                        gen.push("10");
                        gen.m_start << "\tmov rsi, rsp" << "\n";
                        gen.m_start << "\tmov rdx, 1" << "\n";
                        gen.m_start << "\tsyscall" << "\n";
                        gen.m_start << "\tadd rsp, 8\n";
                        gen.m_stack_size--;
                    }
                }
                else {
                    gen.m_start << "\tmov rax, 1" << "\n";
                    gen.m_start << "\tmov rdi, 1" << "\n";
                    gen.push("10");
                    gen.m_start << "\tmov rsi, rsp" << "\n";
                    gen.m_start << "\tmov rdx, 1" << "\n";
                    gen.m_start << "\tsyscall" << "\n";
                    gen.m_start << "\tadd rsp, 8\n";
                    gen.m_stack_size--;
                }
            }
            void operator() (const NodeStmtMethod* node_method) const {
                gen.m_func << node_method->ident.Value.value() << ":\n";

                gen.m_func << "\tleave\n";
                gen.m_func << "\tret\n";
            }
        };


        StmtVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::stringstream gen_prog() {
        this->m_func << "len_msg:\n";
        this->m_func << "\tmov rdx, rsi" << "\n";
        this->m_func << "\t.len_loop:\n";
        this->m_func << "\tcmp byte [rdx], 0\n";
        this->m_func << "\tje .len_done\n";
        this->m_func << "\tinc rdx\n";
        this->m_func << "\tjmp .len_loop\n\n";

        this->m_func << "\t.len_done:\n";
        this->m_func << "\tsub rdx, rsi\n";
        this->m_func << "\tret\n\n";


        this->m_start << "global _start" << "\n";
        this->m_start << "_start:" << "\n";
        for (const NodeStmt* stmt : m_prog->stmts) {
            gen_stmt(stmt);
        }
        this->m_start << "\tmov rax, 60" << "\n";
        this->m_start << "\t" << "mov rdi, 0\n";
        this->m_start << "\tsyscall" << "\n";

        this->m_func << "\n\n";
        this->m_func << "section .text" << std::endl;
        this->m_func << this->m_start.str();
        return std::move(m_func);
    }


private:

    void begin_scope() {
        m_scopes.push_back(m_vars.size());
    }
    void end_scope() {
        const size_t pop_count = m_vars.size() - m_scopes.back();
        m_start << "\tadd rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count;
        for (int a = 0; a < pop_count; a++) {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }
    void push(const std::string& reg) {
        this->m_start << "\tpush " << reg << "\n";
        m_stack_size++;
    }

    void pop (const std::string& reg) {
        this->m_start << "\tpop " << reg << "\n";
        m_stack_size--;
    }
    std::string create_label() {
        return "Label" + std::to_string(label_count++);
    }

    struct Var {
        std::string ident;
        size_t stack_loc;
    };

    struct String {
        std::optional<std::string> ident;
        size_t start_loc, end_loc;
    };

    int label_count = 0;
    std::stringstream m_start;
    std::stringstream m_func;
    const NodeProg* m_prog;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars {};
    std::vector< size_t > m_scopes {};
    std::vector<String> m_string;
};
