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

```Flit
let x = (10 - 2 * 3) / 2 + 10; // Supports PEMDAS Ordering
let y = x + 5; // Supports operation on variable

// has scope for variables
{
    let z = 10; // this will expire after scope ends
    
    // supports if statements
    if (0) {
        print(5); // won't run
    }

    if (1){
        print(999); // will run
    }

    print(z); // prints 10
}

let z = 1000; // as the previous scope expired we can again inititalize z

print(x); // prints 12
print(y); // prints 17
print(z); // prints 1000
```

## Contributing:
Contributions are welcome! Feel free to open issues or submit pull requests.

## Roadmap:

* Add support for 'elif' statements and loops.
* Add Functions
* Introduce more data types.
