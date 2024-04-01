#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include "tokenization.h"

// parser: this function will convert the tokens to assembly code which will be inside a string
std::string tokens_to_asm(const std::vector<Token> &tokens) {

    std::stringstream output;
    output << "global _start\n_start:\n"; // initializing the stringstream with starter code

    for (int i = 0; i < tokens.size(); i++) {

        const Token &token = tokens.at(i);

        if (token.type == TokenType::exit) {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
                    // for this type of code: "exit 10; it will convert it to assembly"
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
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
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