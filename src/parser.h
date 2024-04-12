#pragma once

#include <variant>

#include "tokenizer.h"
#include "arena.h"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr;

struct BinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct BinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<BinExprAdd *, BinExprMulti *> var; // var can be any of the types specified inside the variant<> similar to enum
};

struct NodeExpr {
    std::variant<NodeExprIntLit *, NodeExprIdent *, NodeBinExpr *> var;
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


public:
    inline explicit Parser(std::vector<Token> tokens) :
            m_tokens(std::move(tokens)),
            m_allocator(1024 * 1024 * 4) // 4mb
    {
    }

    // parses the expression
    std::optional<NodeExpr *> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) { // if integer
            auto expr_int_lit = m_allocator.alloc<NodeExprIntLit>();
            expr_int_lit->int_lit = consume();

            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = expr_int_lit;
            return expr;
        } else if (peek().has_value() && peek().value().type == TokenType::ident) { // if identifier
            auto expr_ident = m_allocator.alloc<NodeExprIdent>();
            expr_ident->ident = consume();

            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = expr_ident;
            return expr;
        } else { // otherwise
            return {};
        }
    }

    // parse the statement
    std::optional<NodeStmt *> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek().has_value() &&
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

            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume(); // consume the close parenthesis
            } else {
                std::cerr << "Expected a ')' " << std::endl;
                exit(EXIT_FAILURE);
            }

            // after int_lit we need a semicolon
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "Expected a ';' " << std::endl;
                exit(EXIT_FAILURE);
            }

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

            // check if there is a semicolon
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume(); // consume the semicolon
            } else {
                std::cerr << "Expected a ;" << std::endl;
                exit(EXIT_FAILURE);
            }

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