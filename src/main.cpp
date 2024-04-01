#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

// These are all our token types
enum class TokenType {
    _return,
    int_lit,
    semi
};

struct Token {
    TokenType type;
    std::optional<std::string> value{}; // this is optional
};

std::vector<Token> tokenize(const std::string &str) {

    std::vector<Token> tokens; // this will store all the tokens in order
    std::string buff; // temporary string buffer to store the current token

    for (int i = 0; i < str.length(); i++) {
        char c = str.at(i);

        // as the token can't start with numeric we will start with alpha only
        if (std::isalpha(c)) {
            buff.push_back(c);
            i++;

            // push the character to buffer until there is alphanumeric after it.
            while (std::isalnum(str.at(i))) {
                buff.push_back(str.at(i));
                i++;
            }
            i--;

            // if the buffer value is certain token type
            if (buff == "return") {
                // push the token to vector and clear the buffer
                tokens.push_back({.type = TokenType::_return});
                buff.clear();
                continue;
            } else { // there is syntax error in the code
                std::cerr << "Syntax Error in the Code. Fix it and Try again!" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(c)) { // if token is starting with a number then it must an integer literal
            buff.push_back(c);
            i++;

            // iterate until int_lit isn't finished
            while (std::isdigit(str.at(i))) {
                buff.push_back(str.at(i));
                i++;
            }
            i--;

            // push the int_lit to tokens and give it value of buffer
            tokens.push_back({.type = TokenType::int_lit, .value = buff});
            buff.clear();
        } else if (c == ';') { // push the semicolon to the tokens vector
            tokens.push_back({.type = TokenType::semi});
        } else if (isspace(c)) { // ignore the space
            continue;
        } else {
            // some syntax error has happened
            std::cerr << "Syntax Error in the Code. Fix it and Try again!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

int main(int argc, char *argv[]) {

    // if there are no arguments then throw error
    if (argc != 2) {
        std::cerr << "Incorrect Usage. Correct Usage is:" << std::endl;
        std::cerr << "flit <input.flt>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents; // this string will contain all the input file data
    {
        // read the input file into contents
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);

    return EXIT_SUCCESS;
}