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

// tokenizer: this function will read the string and make a vector with all the tokens
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

// parser: this function will convert the tokens to assembly code which will be inside a string
std::string tokens_to_asm(const std::vector<Token> &tokens) {

    std::stringstream output;
    output << "global _start\n_start:\n"; // initializing the stringstream with starter code

    for (int i = 0; i < tokens.size(); i++) {

        const Token &token = tokens.at(i);

        if (token.type == TokenType::_return) {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
                    // for this type of code: "return 10; it will convert it to assembly"
                    output << "    mov rax, 60\n"; // syscall 60 for sys_exit
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall\n";
                }
            }
        }
    }
    return output.str();
}

int main(int argc, char *argv[]) {

    // if there are no arguments then throw error
    if (argc != 2) {
        std::cerr << "Incorrect Usage. Correct Usage is:" << std::endl;
        std::cerr << "flit <input.flt>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents; // this string will contain all the input file data
    // we are using this brackets to define scope so that it will automatically close the file when the scope ends
    {
        // read the input file into contents
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    } // after this bracket the file will be closed

    // now lets get all the tokens
    std::vector<Token> tokens = tokenize(contents);
    {
        // this will make an output file with assembly code
        std::fstream file("out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }

    // make the output file of assembly (machine code)
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}