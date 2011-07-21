//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ByteCodeFileWriter.hpp"

using std::string;
using std::ios;
using std::endl;

using namespace eddic;

void ByteCodeFileWriter::open(const std::string& path) {
    m_stream.open(path.c_str());

    if (!m_stream) {
        throw CompilerException("Unable to open the output file");
    }
}

void ByteCodeFileWriter::close() {
    m_stream.close();
}

void ByteCodeFileWriter::writeHeader() {
    m_stream << ".text" << endl
             << ".globl main" << endl
             << endl
             << "main:" << endl;
}

void writePrintString(std::ofstream& m_stream) {
    m_stream << std::endl;
    m_stream << "print_string:" << std::endl;
    m_stream << "pushl %ebp" << std::endl;
    m_stream << "movl %esp, %ebp" << std::endl;
    m_stream << "movl $0, %esi" << std::endl;

    m_stream << "movl $4, %eax" << std::endl;
    m_stream << "movl $1, %ebx" << std::endl;
    m_stream << "movl 12(%ebp), %ecx" << std::endl;
    m_stream << "movl 8(%ebp), %edx" << std::endl;
    m_stream << "int $0x80" << std::endl;

    m_stream << "leave" << std::endl;
    m_stream << "ret" << std::endl;
}

void writePrintLine(std::ofstream& m_stream) {
    m_stream << std::endl;
    m_stream << "print_line:" << std::endl;
    m_stream << "pushl %ebp" << std::endl;
    m_stream << "movl %esp, %ebp" << std::endl;

    m_stream << "pushl $S1" << endl;
    m_stream << "pushl $1" << endl;
    m_stream << "call print_string" << endl;
    m_stream << "addl $8, %esp" << endl;

    m_stream << "leave" << std::endl;
    m_stream << "ret" << std::endl;
}

void writePrintInteger(std::ofstream& m_stream) {
    m_stream << std::endl;
    m_stream << "print_integer:" << std::endl
             << "pushl %ebp" << std::endl
             << "movl %esp, %ebp" << std::endl
             << "movl 8(%ebp), %eax" << std::endl
             << "xorl %esi, %esi" << std::endl

             << "loop:" << std::endl
             << "movl $0, %edx" << std::endl
             << "movl $10, %ebx" << std::endl
             << "divl %ebx" << std::endl
             << "addl $48, %edx" << std::endl
             << "pushl %edx" << std::endl
             << "incl %esi" << std::endl
             << "cmpl $0, %eax" << std::endl
             << "jz   next" << std::endl
             << "jmp loop" << std::endl

             << "next:" << std::endl
             << "cmpl $0, %esi" << std::endl
             << "jz   exit" << std::endl
             << "decl %esi" << std::endl

             << "movl $4, %eax" << std::endl
             << "movl %esp, %ecx" << std::endl
             << "movl $1, %ebx" << std::endl
             << "movl $1, %edx" << std::endl
             << "int  $0x80" << std::endl

             << "addl $4, %esp" << std::endl

             << "jmp  next" << std::endl

             << "exit:" << std::endl
             << "movl %ebp, %esp " << std::endl
             << "popl (%ebp)" << std::endl
             << "ret" << std::endl;
}

void ByteCodeFileWriter::writeEnd() {
    //TODO Write only if necessary
    writePrintString(m_stream);
    writePrintInteger(m_stream);
    writePrintLine(m_stream);
}
