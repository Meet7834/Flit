#pragma once

#include <variant>

#include "tokenizer.h"
#include "arena.h"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

// struct NodeBinExprMulti {
//    NodeExpr *lhs;
//    NodeExpr *rhs;
//};

struct NodeBinExpr {
    NodeBinExprAdd *add; // var can be any of the types specified inside the variant<> similar to enum
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident; // identifier
    NodeExpr *expr; // expression
};

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts; // will contain all the statements
};

class Parser {
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;

    // nodiscard means compiler will give a warning if the return value isn't stored/used as it's a constant method. PS: it was suggested by CLion LOL :)
    // basically if you are not using the return value then it's not doing anything hence [[nodiscard]] (you can't discard the return value)
    [[nodiscard]] std::optional<Token> peek(int offset = 0) const {
        // it's a constant method which means it isn't modifying any of its members, so it makes it's only useful for returning value
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    // returns the current element and then increases index
    Token consume() {
        return m_tokens.at(m_index++);
    }

    Token try_consume(TokenType type, const std::string &err_msg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            return {};
        }
    }


public:
    inline explicit Parser(std::vector<Token> tokens) :
            m_tokens(std::move(tokens)),
            m_allocator(1024 * 1024 * 4) // 4mb
    {
    }


    std::optional<NodeTerm *> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) { // if integer
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } else if (auto ident = try_consume(TokenType::ident)) { // if identifier
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else {
            return {};
        }
    }

    // parses the expression
    std::optional<NodeExpr *> parse_expr() {
        if (auto term = parse_term()) {
            if (try_consume(TokenType::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();

                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;

                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                } else {
                    std::cerr << "Expected an Expression!" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value();
                return expr;
            }
        } else {
            return {};
        }
    }

    // parse the statement
    std::optional<NodeStmt *> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek(1).has_value() &&
            peek(1).value().type == TokenType::open_paren) {
            consume(); // consume exit token
            consume(); // consume open parenthesis

            auto *stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                std::cerr << "Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_paren, "Expected a ')'");
            try_consume(TokenType::semi, "Expected a ';'");

            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_exit;
            return node_stmt;
        }
            // for this type of expression:  let x = 5;
        else if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value() &&
                 peek(1).value().type == TokenType::ident && peek(2).has_value() &&
                 peek(2).value().type == TokenType::eq) {

            consume(); // consumes let token
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume(); // consumes variable name
            consume(); // consumes equal sign

            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                std::cerr << "Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            try_consume(TokenType::semi, "Expected a ';'");

            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_let;
            return node_stmt;
        } else {
            return {};
        }
    }

// parse whole program
    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "Invalid Statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        return prog;
    }

};