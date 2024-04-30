#pragma once

namespace gen {

    void genSectionBSS(std::stringstream &m_output) {
        m_output << "\nsection .bss\n";
        m_output << "    digitSpace resb 100\n";
        m_output << "    digitSpacePos resb 8\n";
    }

    void genHeader(std::stringstream &m_output) {
        genSectionBSS(m_output);
        m_output << "\nsection .text\n";
        m_output << "    global _start\n";
    }

    void genPrintRAX(std::stringstream &m_output) {
        m_output << "\n_printRAX:\n";
        m_output << "    mov rcx, digitSpace\n";
        m_output << "    mov rbx, 10\n";
        m_output << "    mov [rcx], rbx\n";
        m_output << "    inc rcx\n";
        m_output << "    mov [digitSpacePos], rcx\n";
    }

    void genPrintRAXLoop(std::stringstream &m_output) {
        m_output << "\n_printRAXLoop:\n";
        m_output << "    mov rdx, 0 ; clear rdx before division\n";
        m_output << "    mov rbx, 10\n";
        m_output << "    div rbx\n";
        m_output << "    push rax\n";
        m_output << "    add rdx, 48 ; convert the remainder to ASCII\n\n";

        m_output << "    mov rcx, [digitSpacePos]\n";
        m_output << "    mov [rcx], dl\n";
        m_output << "    inc rcx\n";
        m_output << "    mov [digitSpacePos], rcx\n\n";

        m_output << "    pop rax\n";
        m_output << "    cmp rax, 0\n";
        m_output << "    jne _printRAXLoop\n";
    }

    void genPrintRAXLoop2(std::stringstream &m_output) {
        m_output << "\n_printRAXLoop2:\n";
        m_output << "    mov rcx, [digitSpacePos]\n\n";

        m_output << "    mov rax, 1\n";
        m_output << "    mov rdi, 1\n";
        m_output << "    mov rsi, rcx\n";
        m_output << "    mov rdx, 1\n";
        m_output << "    syscall\n\n";

        m_output << "    mov rcx, [digitSpacePos]\n";
        m_output << "    dec rcx\n";
        m_output << "    mov [digitSpacePos], rcx\n\n";

        m_output << "    cmp rcx, digitSpace\n";
        m_output << "    jge _printRAXLoop2\n\n";

        m_output << "    ret\n";
    }

    void genFooter(std::stringstream &m_output) {
        genPrintRAX(m_output);
        genPrintRAXLoop(m_output);
        genPrintRAXLoop2(m_output);
    }
}
