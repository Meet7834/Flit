#pragma once

#include "tokenizer.h"

struct NodeExpr {
    Token int_lit;
};

struct NodeExit {
    NodeExpr expr;
};

class Parser {
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    // nodiscard means compiler will give a warning if the return value isn't stored/used as it's a constant method. PS: it was suggested by CLion LOL :)
    // basically if you are not using the return value then it's not doing anything hence [[nodiscard]]
    [[nodiscard]] inline std::optional<Token> peek(int ahead = 1) const {
        // it's a constant method which means it isn't modifying any of its members, so it makes it's only useful for returning value

        if (m_index + ahead > m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index);
        }
    }

    // returns the current element and then increases index
    inline Token consume() {
        return m_tokens.at(m_index++);
    }

public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {

    }

    std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            return NodeExpr{.int_lit = consume()};
        } else {
            return {};
        }
    }

    std::optional<NodeExit> parse() {
        std::optional<NodeExit> exit_node;

        while (peek().has_value()) {
            // for exit token
            if (peek().value().type == TokenType::exit) {
                consume();
                // if the node_expr doesn't have a value after parsing the expression then we will go with the else block
                if (auto node_expr = parse_expr()) {
                    exit_node = NodeExit{.expr = node_expr.value()};
                } else {
                    std::cerr << "Invalid Expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                if (peek().has_value() && peek().value().type == TokenType::semi) {
                    consume();
                    continue;
                } else {
                    std::cerr << "Invalid Expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        m_index = 0;
        return exit_node;
    }
};