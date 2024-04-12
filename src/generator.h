#pragma once

#include <unordered_map>
#include <cassert>
#include "parser.h"

class Generator {
private:
    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;

    struct Var {
        size_t stack_loc;
    };
    std::unordered_map<std::string, Var> m_vars{};

    void push(const std::string &reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string &reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size++;
    }

public:
    // this constructor moves the given argument to private member root
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {

    }

    void gen_expr(const NodeExpr *expr) {
        // this visitor will direct the input to whatever statement we need to generate
        struct ExprVisitor {
            Generator *gen;

            void operator()(const NodeExprIntLit *expr_int_lit) {
                gen->m_output << "    mov rax, " << expr_int_lit->int_lit.value.value() << "\n";
                gen->push("rax");
            }

            void operator()(const NodeExprIdent *expr_ident) {

                // if the given identifier doesn't exist
                if (!gen->m_vars.contains(expr_ident->ident.value.value())) {
                    std::cerr << "Undeclared Identifier " << expr_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                const auto &var = gen->m_vars.at(expr_ident->ident.value.value());
                // we will find the offset and then copy the variable to the top of the stack
                std::stringstream offset;
                // we are multiplying by 8 to convert the integer 64 bits to binary
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }

            void operator()(const NodeBinExpr *bin_expr) const {
                assert(false); // TODO
            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt *stmt) {
        // this visitor will direct the input to whatever statement we need to generate
        struct StmtVisitor {
            Generator *gen;

            void operator()(const NodeStmtExit *stmt_exit) const {
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtLet *stmt_let) {
                if (gen->m_vars.contains(stmt_let->ident.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_let->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen->m_vars.insert({stmt_let->ident.value.value(), Var{.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_let->expr);
            }

            void operator()(const NodeBinExpr *bin_expr) {
                assert(false); // TODO
            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);

    }

    std::string gen_prog() {
        m_output << "global _start\n_start:\n"; // initializing the stringstream with starter code

        for (const NodeStmt *stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }

        // to exit the program (if the user hasn't included exit statement)
        m_output << "    mov rax, 60\n"; // syscall 60 for sys_exit
        m_output << "    mov rdi, 0\n"; // return 0
        m_output << "    syscall\n";

        return m_output.str();
    }
};