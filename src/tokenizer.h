#pragma once

#include <string>
#include <vector>

// These are all our token types
enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    print,
    plus,
    multi,
    minus,
    div,
    open_curly,
    close_curly,
    if_,
    elif,
    else_
};

std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::multi:
        case TokenType::div:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value{}; // this is optional
};

class Tokenizer {

private:
    const std::string m_src;
    size_t m_index = 0; // size_t is basically and unsigned 32-bit integer

    // nodiscard means compiler will give a warning if the return value isn't stored/used as it's a constant method. PS: it was suggested by CLion LOL :)
    // basically if you are not using the return value then it's not doing anything hence [[nodiscard]]
    [[nodiscard]] std::optional<char> peek(int offset = 0) const {
        // it's a constant method which means it isn't modifying any of its members, so it makes it's only useful for returning value
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
        // this constructor just moves the value of string in our private member src
    }

    // tokenizer: this function will read the string and make a vector with all the tokens
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        std::string buff; // temporary string buffer to store the current token

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
                    tokens.push_back({.type = TokenType::exit});
                    buff.clear();
                } else if (buff == "let") {
                    tokens.push_back({.type = TokenType::let});
                    buff.clear();
                } else if (buff == "print") {
                    tokens.push_back({.type = TokenType::print});
                    buff.clear();
                } else if (buff == "if") {
                    tokens.push_back({.type = TokenType::if_});
                    buff.clear();
                } else if (buff == "elif") {
                    tokens.push_back({.type = TokenType::elif});
                    buff.clear();
                } else if (buff == "else") {
                    tokens.push_back({.type = TokenType::else_});
                    buff.clear();
                } else { // if it's not a keyword then make it an identifier
                    tokens.push_back({.type = TokenType::ident, .value = buff});
                    buff.clear();
                }
            } else if (std::isdigit(peek().value())) {
                // if token is starting with a number then it must an integer literal
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
                    consume();
                }
                if (peek().has_value()) consume(); // consume '*'
                if (peek().has_value()) consume(); // consume '/'
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_paren});
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::close_paren});
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
            } else if (peek().value() == '-') {
                consume();
                tokens.push_back({.type = TokenType::minus});
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::multi});
            } else if (peek().value() == '/') {
                consume();
                tokens.push_back({.type = TokenType::div});
            } else if (peek().value() == '{') {
                consume();
                tokens.push_back({.type = TokenType::open_curly});
            } else if (peek().value() == '}') {
                consume();
                tokens.push_back({.type = TokenType::close_curly});
            } else if (isspace(peek().value())) {
                consume();
            } else {
                // some syntax error has happened
                std::cerr << "Syntax Error in the Code. Fix it and Try again!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        m_index = 0; // in-case we need to use this again
        return tokens;
    }
};