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

std::string to_string(const TokenType type) {
    switch (type) {
        case TokenType::exit:
            return "`exit`";
        case TokenType::int_lit:
            return "int literal";
        case TokenType::semi:
            return "`;`";
        case TokenType::open_paren:
            return "`(`";
        case TokenType::close_paren:
            return "`)`";
        case TokenType::ident:
            return "identifier";
        case TokenType::let:
            return "`let`";
        case TokenType::eq:
            return "`=`";
        case TokenType::plus:
            return "`+`";
        case TokenType::multi:
            return "`*`";
        case TokenType::minus:
            return "`-`";
        case TokenType::div:
            return "`/`";
        case TokenType::open_curly:
            return "`{`";
        case TokenType::close_curly:
            return "`}`";
        case TokenType::if_:
            return "`if`";
        case TokenType::elif:
            return "`elif`";
        case TokenType::else_:
            return "`else`";
        case TokenType::print:
            return "`print`";
    }
    assert(false);
}

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
    int line;
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
                } else { // if it's not a keyword then make it an identifier
                    tokens.push_back({TokenType::ident, line_count, buff});
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
                // some syntax error has happened
                std::cerr << "Syntax Error in the Code. Fix it and Try again!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        m_index = 0; // in-case we need to use this again
        return tokens;
    }
};