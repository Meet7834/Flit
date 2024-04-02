#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

#include "tokenizer.h"
#include "parser.h"
#include "generator.h"

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

    Parser parser(std::move(tokens));

    std::optional<NodeExit> tree = parser.parse();
    if (!tree.has_value()) {
        std::cerr << "No exit statement found!" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());
    {
        // this will make an output file with assembly code
        std::fstream file("out.asm", std::ios::out);
        file << generator.generate();
    }

    // execute the assembly code and make an output file from it (machine code)
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}