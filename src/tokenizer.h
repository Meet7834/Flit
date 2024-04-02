#pragma once

#include <string>
#include <vector>

// These are all our token types
enum class TokenType {
    exit, int_lit, semi
};

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
    [[nodiscard]] inline std::optional<char> peek(int ahead = 1) const {
        // it's a constant method which means it isn't modifying any of its members, so it makes it's only useful for returning value

        if (m_index + ahead > m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index);
        }
    }

    // returns the current element and then increases index
    inline char consume() {
        return m_src.at(m_index++);
    }

public:
    // explicit because it shouldn't accidentally convert string into a tokenizer
    inline explicit Tokenizer(std::string src) : m_src(std::move(src)) {
        // this constructor just moves the value of string in our private member src
    }

    // tokenizer: this function will read the string and make a vector with all the tokens
    inline std::vector<Token> tokenize() {
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
                    continue;
                } else { // there is syntax error in the code
                    std::cerr << "Syntax Error in the Code. Fix it and Try again!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                continue;
            } else if (std::isdigit(peek().value())) {
                // if token is starting with a number then it must an integer literal
                buff.push_back(consume());

                while (peek().has_value() && std::isdigit(peek().value())) {
                    buff.push_back(consume());
                }

                // push the int_lit to tokens and give it value of buffer
                tokens.push_back({.type = TokenType::int_lit, .value = buff});
                buff.clear();
                continue;
            } else if (peek().value() == ';') {
                tokens.push_back({.type = TokenType::semi});
                consume();
                continue;
            } else if (isspace(peek().value())) {
                consume();
                continue;
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