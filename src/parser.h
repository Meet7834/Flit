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

struct NodeTermParen {
    NodeExpr *expr;
};

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMinus {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd *, NodeBinExprMinus *, NodeBinExprMulti *, NodeBinExprDiv *> var; // var can be any of the types specified inside the variant<> similar to enum
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen *> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtPrint {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident; // identifier
    NodeExpr *expr; // expression
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt *> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr *expr;
    NodeScope *scope;
    std::optional<NodeIfPred *> pred;
};

struct NodeIfPredElse {
    NodeScope *scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif *, NodeIfPredElse *> var;
};

struct NodeStmtIf {
    NodeExpr *expr;
    NodeScope *scope;
    std::optional<NodeIfPred *> pred;
};

struct NodeStmtWhile {
    NodeExpr *expr;
    NodeScope *scope;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *, NodeStmtPrint *, NodeScope *, NodeStmtIf *, NodeStmtAssign *, NodeStmtWhile *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts; // will contain all the statements
};

class Parser {
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;

    [[nodiscard]] std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens.at(m_index + offset);
    }

    // returns the current element and then increases index
    Token consume() {
        return m_tokens.at(m_index++);
    }

    // for better error handling
    Token try_consume_err(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(to_string(type));
        return {};
    }

    std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }

public:
    explicit Parser(std::vector<Token> tokens) :
            m_tokens(std::move(tokens)),
            m_allocator(1024 * 1024 * 4) // 4mb
    {
    }

    void error_expected(const std::string &msg) {
        std::cerr << "[Parsing Error] Expected `" << msg << "` on line " << peek(-1).value().line << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<NodeTerm *> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) { // if integer
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        if (auto ident = try_consume(TokenType::ident)) { // if identifier
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        if (auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);

            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;

            return term;
        }
        return {};
    }

    std::optional<NodeExpr *> parse_expr(int min_prec = 0) {

        std::optional<NodeTerm *> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true) {
            std::optional<Token> curr_token = peek();
            std::optional<int> prec;

            if (curr_token.has_value()) {
                prec = bin_prec(curr_token->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else {
                break;
            }

            Token op = consume();
            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                error_expected("expression");
            }

            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.alloc<NodeExpr>();

            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs2->var = expr_lhs->var;

                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            } else if (op.type == TokenType::minus) {
                auto sub = m_allocator.alloc<NodeBinExprMinus>();
                expr_lhs2->var = expr_lhs->var;

                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
            } else if (op.type == TokenType::multi) {
                auto multi = m_allocator.alloc<NodeBinExprMulti>();
                expr_lhs2->var = expr_lhs->var;

                multi->lhs = expr_lhs2;
                multi->rhs = expr_rhs.value();
                expr->var = multi;
            } else if (op.type == TokenType::div) {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs2->var = expr_lhs->var;

                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
            } else {
                assert(false); // unreachable
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeScope *> parse_scope() {
        if (!try_consume(TokenType::open_curly).has_value()) {
            return {};
        }
        auto scope = m_allocator.alloc<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume_err(TokenType::close_curly);
        return scope;
    }

    std::optional<NodeIfPred *> parse_if_pred() {
        if (try_consume(TokenType::elif)) {
            auto elif_pred = m_allocator.alloc<NodeIfPredElif>();

            try_consume_err(TokenType::open_paren);
            if (auto expr = parse_expr()) {
                elif_pred->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);

            if (auto scope = parse_scope()) {
                elif_pred->scope = scope.value();
            } else {
                error_expected("scope");
            }

            elif_pred->pred = parse_if_pred();
            auto if_pred = m_allocator.alloc<NodeIfPred>();
            if_pred->var = elif_pred;
            return if_pred;
        }
        if (try_consume(TokenType::else_)) {
            auto else_pred = m_allocator.alloc<NodeIfPredElse>();

            if (auto scope = parse_scope()) {
                else_pred->scope = scope.value();
            } else {
                error_expected("scope");
            }

            auto if_pred = m_allocator.alloc<NodeIfPred>();
            if_pred->var = else_pred;
            return if_pred;
        }
        return {};
    }

    std::optional<NodeStmt *> parse_stmt() {
        // for exit token
        if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value() &&
            peek(1).value().type == TokenType::open_paren) {
            consume(); // consume exit token
            consume(); // consume open parenthesis

            auto *stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                error_expected("expression");
            }

            try_consume_err(TokenType::close_paren);
            try_consume_err(TokenType::semi);

            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_exit;
            return node_stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value() &&
            peek(1).value().type == TokenType::ident && peek(2).has_value() &&
            peek(2).value().type == TokenType::eq) {

            consume(); // consumes let token
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume(); // consumes variable name
            consume(); // consumes equal sign

            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                error_expected("expression");
            }

            try_consume_err(TokenType::semi);
            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_let;
            return node_stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value() &&
            peek(1).value().type == TokenType::eq) {

            auto assign_var = m_allocator.alloc<NodeStmtAssign>();
            assign_var->ident = consume();
            consume(); // consume equals token

            if (auto expr = parse_expr()) {
                assign_var->expr = expr.value();
            } else {
                error_expected("Variable Assignment");
            }
            try_consume_err(TokenType::semi);

            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = assign_var;
            return node_stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::print && peek(1).has_value() &&
            peek(1).value().type == TokenType::open_paren) {
            consume(); // consume the print token
            consume(); // consume open parenthesis

            auto stmt_print = m_allocator.alloc<NodeStmtPrint>();
            if (auto node_expr = parse_expr()) {
                stmt_print->expr = node_expr.value();
            } else {
                error_expected("print expression");
            }

            try_consume_err(TokenType::close_paren);
            try_consume_err(TokenType::semi);

            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_print;
            return node_stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = scope.value();
                return stmt;
            }
            error_expected("scope");
        }
        if (try_consume(TokenType::if_)) {

            try_consume_err(TokenType::open_paren);
            auto stmt_if = m_allocator.alloc<NodeStmtIf>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);

            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                error_expected("scope");
            }

            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_if;
            return stmt;
        }
        if (try_consume(TokenType::while_)) {
            try_consume_err(TokenType::open_paren);
            auto stmt_while = m_allocator.alloc<NodeStmtWhile>();
            if (auto expr = parse_expr()) {
                stmt_while->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);

            if (auto scope = parse_scope()) {
                stmt_while->scope = scope.value();
            } else {
                error_expected("scope");
            }

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_while;
            return stmt;
        }
        return {};
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                error_expected("statement");
            }
        }
        return prog;
    }

};