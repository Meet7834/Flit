#pragma once

#include <variant>

#include "tokenizer.h"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprIdent> var; // var can be any of the types specified inside the variant<> similar to enum
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtLet {
    Token ident; // identifier
    NodeExpr expr; // expression
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtLet> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts; // will contain all the statements
};

class Parser {
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    // nodiscard means compiler will give a warning if the return value isn't stored/used as it's a constant method. PS: it was suggested by CLion LOL :)
    // basically if you are not using the return value then it's not doing anything hence [[nodiscard]] (you can't discard the return value)
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
        // it's a constant method which means it isn't modifying any of its members, so it makes it's only useful for returning value
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    // returns the current element and then increases index
    inline Token consume() {
        return m_tokens.at(m_index++);
    }

public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {

    }

    // parses the expression
    std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) { // if integer
            return NodeExpr{.var = NodeExprIntLit{.int_lit = consume()}};
        } else if (peek().has_value() && peek().value().type == TokenType::ident) { // if identifier
            return NodeExpr{.var = NodeExprIdent{.ident = consume()}};
        } else { // otherwise
            return {};
        }
    }

    // parse the statement
    std::optional<NodeStmt> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek().has_value() &&
            peek(1).value().type == TokenType::open_paren) {
            consume(); // consume exit token
            consume(); // consume open parenthesis

            NodeStmtExit stmt_exit;
            if (auto node_expr = parse_expr()) {
                stmt_exit = {.expr = node_expr.value()};
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

            return NodeStmt{.var = stmt_exit};
        }
            // for this type of expression:  let x = 5;
        else if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value() &&
                 peek(1).value().type == TokenType::ident && peek(2).has_value() &&
                 peek(2).value().type == TokenType::eq) {

            consume(); // consumes let token
            auto stmt_let = NodeStmtLet{.ident = consume()}; // consumes variable name
            consume(); // consumes equal sign

            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
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

            return NodeStmt{.var = stmt_let};
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