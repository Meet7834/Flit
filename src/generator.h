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
        m_stack_size--;
    }

    void genSectionBSS() {
        m_output << "\nsection .bss\n";
        m_output << "    digitSpace resb 100\n";
        m_output << "    digitSpacePos resb 8\n";
    }

    void genHeader(){
        genSectionBSS();
        m_output << "\nsection .text\n";
        m_output << "    global _start\n";
    }

    void genPrintRAX() {
        m_output << "\n_printRAX:\n";
        m_output << "    mov rcx, digitSpace\n";
        m_output << "    mov rbx, 10\n";
        m_output << "    mov [rcx], rbx\n";
        m_output << "    inc rcx\n";
        m_output << "    mov [digitSpacePos], rcx\n";
    }

    void genPrintRAXLoop() {
        m_output << "\n_printRAXLoop:\n";
        m_output << "    mov rdx, 0 \n";
        m_output << "    mov rbx, 10\n";
        m_output << "    div rbx\n";
        m_output << "    push rax\n";
        m_output << "    add rdx, 48\n\n";

        m_output << "    mov rcx, [digitSpacePos]\n";
        m_output << "    mov [rcx], dl\n";
        m_output << "    inc rcx\n";
        m_output << "    mov [digitSpacePos], rcx\n\n";

        m_output << "    pop rax\n";
        m_output << "    cmp rax, 0\n";
        m_output << "    jne _printRAXLoop\n";
    }

    void genPrintRAXLoop2() {
        m_output << "\n_printRAXLoop2:\n";
        m_output << "    mov rcx, [digitSpacePos]\n\n";

        m_output << "    mov rax, 1\n";
        m_output << "    mov rdi, 1\n";
        m_output << "    mov rsi, rcx\n";
        m_output << "    mov rdx, 1\n";
        m_output << "    syscall\n\n";

        m_output << "    mov rcx, [digitSpacePos]\n";
        m_output << "    dec rcx\n";
        m_output << "    mov [digitSpacePos], rcx\n\n";

        m_output << "    cmp rcx, digitSpace\n";
        m_output << "    jge _printRAXLoop2\n\n";

        m_output << "    ret\n";
    }

    void genFooter() {
        genPrintRAX();
        genPrintRAXLoop();
        genPrintRAXLoop2();
    }

public:
    // this constructor moves the given argument to private member root
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {

    }

    void gen_term(const NodeTerm *term) {
        struct TermVisitor {
            Generator *gen;

            void operator()(const NodeTermIntLit *termIntLit) const {
                gen->m_output << "    ; adding the integer to the stack\n";
                gen->m_output << "    mov rax, " << termIntLit->int_lit.value.value() << "\n";
                gen->push("rax");
            }

            void operator()(const NodeTermIdent *term_ident) const {
                // if the given identifier doesn't exist
                if (!gen->m_vars.contains(term_ident->ident.value.value())) {
                    std::cerr << "Undeclared Identifier " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen->m_output << "    ; finding the identifier location\n";
                const auto &var = gen->m_vars.at(term_ident->ident.value.value());
                // we will find the offset and then copy the variable to the top of the stack
                std::stringstream offset;
                // we are multiplying by 8 to convert the integer 64 bits to binary
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }
        };

        TermVisitor visitor({.gen = this});
        std::visit(visitor, term->var);
    }

    void gen_expr(const NodeExpr *expr) {
        // this visitor will direct the input to whatever statement we need to generate
        struct ExprVisitor {
            Generator *gen;

            void operator()(const NodeTerm *term) const {
                gen->gen_term(term);
            }

            void operator()(const NodeBinExpr *bin_expr) const {
                gen->m_output << "    ; binary expression\n";

                gen->gen_expr(bin_expr->add->lhs);
                gen->gen_expr(bin_expr->add->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
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
                gen->m_output << "    ; exit statement\n";

                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtLet *stmt_let) const {
                if (gen->m_vars.contains(stmt_let->ident.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_let->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen->m_output << "    ; declaring identifier\n";

                gen->m_vars.insert({stmt_let->ident.value.value(), Var{.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_let->expr);
            }

            void operator()(const NodeStmtPrint *stmt_print) const {
                gen->m_output << "    ; print statement\n";

                gen->gen_expr(stmt_print->expr);
                gen->m_output << "    pop rax\n";
                gen->m_output << "    call _printRAX\n\n";
            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    std::string gen_prog() {
        // ads bss section to the top of the assembly code
        genHeader();

        m_output << "\n_start:\n"; // initializing the stringstream with starter code

        for (const NodeStmt *stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }

        m_output << "    ; exiting the program\n";

        // to exit the program (in case the user hasn't included exit statement)
        m_output << "    mov rax, 60\n"; // syscall 60 for sys_exit
        m_output << "    mov rdi, 0\n"; // return 0
        m_output << "    syscall\n";

        // generates assembly code for printing rax function
        genFooter();

        return m_output.str();
    }
};