# Flit
*A compiler for the Flit programming language.*

## Features

* **Lexical Analysis:** Breaks down Flit code into meaningful tokens (keywords, identifiers, numbers, etc.).
* **Syntax Analysis:** Constructs an Abstract Syntax Tree (AST) representing the structure of Flit programs.
* **Code Generation:** Translates the AST into x86-64 assembly code.
* **Memory Allocator:** Allocates memory linearly in previous reserved chunk
* **Project Workflow:** Tokenize => Parse => Generate Assembly Code 
* **Operator Precedence:** PEMDAS precedence for arithmetic operations implemented.

## Usage Instructions
**Prerequisites**

* NASM (Assembler)
* LD (Linker)
* A suitable operating system (Linux for now)

**Steps**

1.  Clone this repository:
    ```bash
    git clone https://github.com/Meet7834/Flit.git
    ```

2.  Navigate to directory:
    ```bash
    cd flit
    ```

3.  Compile the Flit program:
    ```bash
    mkdir build
    cmake -S . -B build
    cmake --build build 
    ```
4.  Execute the program with:
    ```bash
    ./build/flit ./my_program.flt
    ```
## Example Flit Program
*  For Sample code see grammar.md or see allFeatures.flt or test.flt

## Contributing:
Contributions are welcome! Feel free to open issues or submit pull requests.

## Roadmap:
* Add tests
* Add Comparison Operators (==, <, >, etc...)
* Introduce Functions
* Introduce more data types.
