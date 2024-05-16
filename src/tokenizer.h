#pragma once

#include <string>
#include <vector>
#include "structures/tokens.h"

class Tokenizer {
private:
    const std::string m_src;
    size_t m_index = 0;

    [[nodiscard]] std::optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    }

    // returns the current element and then increases index
    char consume() {
        return m_src.at(m_index++);
    }

public:
    // explicit because it shouldn't accidentally convert string into a tokenizer
    explicit Tokenizer(std::string src) : m_src(std::move(src)) {
    }

    // tokenizer: this function will read the string and make a vector with all the tokens
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        std::string buff;
        int line_count = 1;

        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                // as the token can't start with numeric we will start with alpha only
                buff.push_back(consume());

                while (peek().has_value() && std::isalnum(peek().value())) {
                    buff.push_back(consume()); // push the character to buffer until there is alphanumeric after it.
                }

                // if the buffer value is a keyword
                if (buff == "exit") {
                    // push the token to vector and clear the buffer
                    tokens.push_back({TokenType::exit, line_count});
                    buff.clear();
                } else if (buff == "let") {
                    tokens.push_back({TokenType::let, line_count});
                    buff.clear();
                } else if (buff == "print") {
                    tokens.push_back({TokenType::print, line_count});
                    buff.clear();
                } else if (buff == "if") {
                    tokens.push_back({TokenType::if_, line_count});
                    buff.clear();
                } else if (buff == "elif") {
                    tokens.push_back({TokenType::elif, line_count});
                    buff.clear();
                } else if (buff == "else") {
                    tokens.push_back({TokenType::else_, line_count});
                    buff.clear();
                } else if (buff == "while") {
                    tokens.push_back({TokenType::while_, line_count});
                    buff.clear();
                } else { // if it's not a keyword then make it an identifier
                    tokens.push_back({TokenType::ident, line_count, buff});
                    buff.clear();
                }
            } else if (std::isdigit(peek().value())) {
                // if token is starting with a number then it must be an integer literal
                buff.push_back(consume());

                while (peek().has_value() && std::isdigit(peek().value())) {
                    buff.push_back(consume());
                }

                // push the int_lit to tokens and give it value of buffer
                tokens.push_back({.type = TokenType::int_lit, .value = buff});
                buff.clear();
            } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                // consume both '/'
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
            } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                // consume '/' then '*'
                consume();
                consume();
                while (peek().has_value()) {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                        break;
                    }
                    if (peek().value() == '\n') line_count++;
                    consume();
                }
                if (peek().has_value()) consume(); // consume '*'
                if (peek().has_value()) consume(); // consume '/'
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({TokenType::open_paren, line_count});
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({TokenType::close_paren, line_count});
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({TokenType::semi, line_count});
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({TokenType::eq, line_count});
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({TokenType::plus, line_count});
            } else if (peek().value() == '-') {
                consume();
                tokens.push_back({TokenType::minus, line_count});
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({TokenType::multi, line_count});
            } else if (peek().value() == '/') {
                consume();
                tokens.push_back({TokenType::div, line_count});
            } else if (peek().value() == '{') {
                consume();
                tokens.push_back({TokenType::open_curly, line_count});
            } else if (peek().value() == '}') {
                consume();
                tokens.push_back({TokenType::close_curly, line_count});
            } else if (peek().value() == '\n') {
                consume();
                line_count++;
            } else if (isspace(peek().value())) {
                consume();
            } else {
                // some syntax error happened
                std::cerr << "Unexpected Token on line " << line_count << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        m_index = 0; // resetting index
        return tokens;
    }
};