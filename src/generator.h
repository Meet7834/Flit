#pragma once

#include <cassert>
#include "parser.h"
#include "utils.h"

class Generator {
private:
    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    int m_label_count = 0;

    struct Var {
        std::string name;
        size_t stack_loc;
    };
    std::vector<Var> m_vars{};
    std::vector<size_t> m_scopes{};

    void push(const std::string &reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string &reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    void begin_scope() {
        m_output << "    ; scope begin\n";
        m_scopes.push_back(m_vars.size());
    }

    void end_scope() {
        // find out how many elements to pop whose scope has expired
        int pop_count = m_vars.size() - m_scopes.back();
        m_output << "    ; scope ended\n";

        // increment the location of stack pointer to previous scope location
        m_output << "    add rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count; // decrease the stack size,
        for (int i = 0; i < pop_count; i++) {
            m_vars.pop_back(); // pop all the variables which are expired
        }
        m_scopes.pop_back();
    }

    std::string create_label() {
        std::stringstream ss;
        ss << "label" << m_label_count++;
        return ss.str();
    }

public:
    // this constructor moves the given argument to private member root
    explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {

    }

    void gen_term(const NodeTerm *term) {
        struct TermVisitor {
            Generator &gen;

            void operator()(const NodeTermIntLit *termIntLit) const {
                gen.m_output << "    ; adding the integer to the stack\n";
                gen.m_output << "    mov rax, " << termIntLit->int_lit.value.value() << "\n";
                gen.push("rax");
            }

            void operator()(const NodeTermIdent *term_ident) const {

                // if the given identifier doesn't exist
                auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var &var) {
                    return var.name == term_ident->ident.value.value();
                });
                if (it == gen.m_vars.cend()) {
                    std::cerr << "Undeclared Identifier " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen.m_output << "    ; finding the identifier location\n";
                std::stringstream offset;
                // we will find the offset and then copy the variable to the top of the stack
                // we are multiplying by 8 to convert the integer 64 bits to binary
                offset << "QWORD [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]\n";
                gen.push(offset.str());
            }

            void operator()(const NodeTermParen *term_paren) const {
                gen.gen_expr(term_paren->expr);
            }
        };

        TermVisitor visitor({.gen = *this});
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr *bin_expr) {
        struct BinExprVisitor {
            Generator &gen;

            void operator()(const NodeBinExprAdd *add) const {
                gen.gen_expr(add->rhs);
                gen.gen_expr(add->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    add rax, rbx\n";
                gen.push("rax");
            }

            void operator()(const NodeBinExprMinus *sub) const {
                gen.gen_expr(sub->rhs);
                gen.gen_expr(sub->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    sub rax, rbx\n";
                gen.push("rax");
            }

            void operator()(const NodeBinExprMulti *multi) const {
                gen.gen_expr(multi->rhs);
                gen.gen_expr(multi->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    mul rbx\n";
                gen.push("rax");
            }

            void operator()(const NodeBinExprDiv *div) const {
                gen.gen_expr(div->rhs);
                gen.gen_expr(div->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    mov rdx, 0\n"; // clearing the rdx register before division
                gen.m_output << "    div rbx\n";
                gen.push("rax");
            }
        };
        BinExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr *expr) {
        // this visitor will direct the input to whatever statement we need to generate
        struct ExprVisitor {
            Generator &gen;

            void operator()(const NodeTerm *term) const {
                gen.gen_term(term);
            }

            void operator()(const NodeBinExpr *bin_expr) const {
                gen.m_output << "    ; binary expression\n";
                gen.gen_bin_expr(bin_expr);
            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_scope(const NodeScope *scope) {
        begin_scope();
        for (const NodeStmt *stmt: scope->stmts) {
            gen_stmt(stmt);
        }
        end_scope();
    }

    void gen_if_pred(const NodeIfPred *if_pred, const std::string &end_label) {
        struct PredVisitor {
            Generator &gen;
            const std::string &end_label;

            void operator()(const NodeIfPredElif *elif) const {
                gen.gen_expr(elif->expr);
                gen.pop("rax");
                std::string label = gen.create_label();
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jz " << label << "\n";
                gen.gen_scope(elif->scope);
                gen.m_output << "    jmp " << end_label << "\n";
                if (elif->pred.has_value()) {
                    gen.m_output << label << ":\n";
                    gen.gen_if_pred(elif->pred.value(), end_label);
                }
            }

            void operator()(const NodeIfPredElse *else_) {
                gen.gen_scope(else_->scope);
            }
        };

        PredVisitor visitor{.gen = *this, .end_label = end_label};
        std::visit(visitor, if_pred->var);
    }

    void gen_stmt(const NodeStmt *stmt) {
        // this visitor will direct the input to whatever statement we need to generate
        struct StmtVisitor {
            Generator &gen;

            void operator()(const NodeStmtExit *stmt_exit) const {
                gen.m_output << "    ; exit statement\n";

                gen.gen_expr(stmt_exit->expr);
                gen.m_output << "    mov rax, 60\n";
                gen.pop("rdi");
                gen.m_output << "    syscall\n";
            }

            void operator()(const NodeStmtLet *stmt_let) const {

                auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var &var) {
                    return var.name == stmt_let->ident.value.value();
                });
                if (it != gen.m_vars.cend()) {
                    std::cerr << "Identifier already used: " << stmt_let->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen.m_output << "    ; declaring identifier\n";

                gen.m_vars.push_back({.name = stmt_let->ident.value.value(), .stack_loc = gen.m_stack_size});
                gen.gen_expr(stmt_let->expr);
            }

            void operator()(const NodeStmtPrint *stmt_print) const {
                gen.m_output << "    ; print statement\n";

                gen.gen_expr(stmt_print->expr);
                gen.pop("rax");
                gen.m_output << "    call _printRAX\n";
            }

            void operator()(const NodeScope *scope) const {
                gen.gen_scope(scope);
            }

            void operator()(const NodeStmtIf *stmt_if) const {
                gen.gen_expr(stmt_if->expr);
                gen.pop("rax");
                std::string label = gen.create_label();
                gen.m_output << "    test rax, rax ; if statement\n";
                gen.m_output << "    jz " << label << "\n";
                gen.gen_scope(stmt_if->scope);
                if (stmt_if->pred.has_value()){
                    const std::string end_label = gen.create_label();
                    gen.m_output << "    jmp " << end_label << "\n";
                    gen.m_output << label << ":\n";
                    gen.gen_if_pred(stmt_if->pred.value(), end_label);
                    gen.m_output << end_label << ":\n";
                } else {
                    gen.m_output << label << ":\n";
                }
            }
        };

        // construct visitor instance, it will call the suited method for the variable type
        StmtVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    std::string gen_prog() {
        // ads bss section to the top of the assembly code
        gen::genHeader(m_output);

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
        gen::genFooter(m_output);

        return m_output.str();
    }
};