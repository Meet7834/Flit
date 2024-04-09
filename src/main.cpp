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

    // reading input from a file
    std::string contents;
    // we are using this brackets to define scope so that it will automatically close the file when the scope ends
    {
        // read the input file into contents
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    } // after this bracket the file will be automatically closed because of scope resolution

    // now we will generate token for the input file
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    // now lets parse all the tokens
    Parser parser(std::move(tokens));

    // make a root node of tree for parser
    std::optional<NodeProg> prog = parser.parse_prog();
    if (!prog.has_value()) {
        std::cerr << "Invalid Code! Please fix the error and try again." << std::endl;
        exit(EXIT_FAILURE);
    }

    // generate assembly code based using root node of the parse tree
    Generator generator(prog.value());
    {
        // this will make an output file with assembly code
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    // execute the assembly code and make an output file from it (machine code)
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}