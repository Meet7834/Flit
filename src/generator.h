#pragma once

#include "parser.h"

class Generator {
private:
    const NodeExit m_root;

public:
    inline explicit Generator(NodeExit root) : m_root(std::move(root)) {

    }

    [[nodiscard]] inline const std::string generate() {
        std::stringstream output;
        output << "global _start\n_start:\n"; // initializing the stringstream with starter code

        // for this type of code: "exit 10; it will convert it to assembly"
        output << "    mov rax, 60\n"; // syscall 60 for sys_exit
        output << "    mov rdi, " << m_root.expr.int_lit.value.value() << "\n";
        output << "    syscall\n";

        return output.str();
    }
};