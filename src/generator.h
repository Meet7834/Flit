#pragma once

#include "parser.h"

class Generator {
private:
    const NodeProg m_prog;
    std::stringstream m_output;

public:
    // this constructor moves the given argument to private member root
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {

    }

    void gen_expr(const NodeExpr &expr) {
        // this visitor will direct the input to whatever statement we need to generate
        struct ExprVisitor {
            Generator *gen;

            void operator()(const NodeExprIntLit &expr_int_lit) {
                gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
                gen->m_output << "    push rax \n";
            }

            void operator()(const NodeExprIdent &expr_ident) {
                // TODO
            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr.var);
    }

    void gen_stmt(const NodeStmt &stmt) {
        // this visitor will direct the input to whatever statement we need to generate
        struct StmtVisitor {
            Generator *gen;

            void operator()(const NodeStmtExit &stmt_exit) const {
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "    mov rax, 60\n";
                gen->m_output << "    pop rdi\n"; // pop off the stack into the rdi register
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtLet &stmt_let) {

            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt.var);

    }

    std::string gen_prog() {
        m_output << "global _start\n_start:\n"; // initializing the stringstream with starter code

        for (const NodeStmt &stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }

        // to exit the program (if the user hasn't included exit statement)
        m_output << "    mov rax, 60\n"; // syscall 60 for sys_exit
        m_output << "    mov rdi, 0\n"; // return 0
        m_output << "    syscall\n";

        return m_output.str();
    }
};