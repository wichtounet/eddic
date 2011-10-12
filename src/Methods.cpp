//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "AssemblyFileWriter.hpp"
#include "Methods.hpp"

using namespace eddic;

static void writePrintString(std::ofstream& m_stream) {
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

static void writePrintLine(std::ofstream& m_stream) {
    m_stream << std::endl;
    m_stream << "print_line:" << std::endl;
    m_stream << "pushl %ebp" << std::endl;
    m_stream << "movl %esp, %ebp" << std::endl;

    m_stream << "pushl $S1" << std::endl;
    m_stream << "pushl $1" << std::endl;
    m_stream << "call print_string" << std::endl;
    m_stream << "addl $8, %esp" << std::endl;

    m_stream << "leave" << std::endl;
    m_stream << "ret" << std::endl;
}

static void writePrintInteger(std::ofstream& m_stream) {
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
             << "leave" << std::endl
             << "ret" << std::endl;
}

static void writeConcat(std::ofstream& m_stream){
    m_stream << std::endl;
    m_stream << "concat:" << std::endl
        << "pushl %ebp" << std::endl
        << "movl %esp, %ebp" << std::endl

        << "movl 16(%ebp), %edx" << std::endl
        << "movl 8(%ebp), %ecx" << std::endl
        << "addl %ecx, %edx" << std::endl

        << "pushl %edx" << std::endl
        << "call malloc" << std::endl
        << "addl $4, %esp" << std::endl

        << "movl %eax, -4(%ebp)" << std::endl
        << "movl %eax, %ecx" << std::endl
        << "movl $0, %eax" << std::endl

        << "movl 16(%ebp), %ebx" << std::endl
        << "movl 20(%ebp), %edx" << std::endl

        << "copy_concat_1:" << std::endl
        << "cmpl $0, %ebx" << std::endl
        << "je end_concat_1"  << std::endl
        << "movb (%edx), %al" << std::endl
        << "movb %al, (%ecx)" << std::endl
        << "addl $1, %ecx" << std::endl
        << "addl $1, %edx" << std::endl
        << "subl $1, %ebx" << std::endl
        << "jmp copy_concat_1" << std::endl
        << "end_concat_1" << ":" << std::endl

        << "movl 8(%ebp), %ebx" << std::endl
        << "movl 12(%ebp), %edx" << std::endl

        << "copy_concat_2:" << std::endl
        << "cmpl $0, %ebx" << std::endl
        << "je end_concat_2"  << std::endl
        << "movb (%edx), %al" << std::endl
        << "movb %al, (%ecx)" << std::endl
        << "addl $1, %ecx" << std::endl
        << "addl $1, %edx" << std::endl
        << "subl $1, %ebx" << std::endl
        << "jmp copy_concat_2" << std::endl
        << "end_concat_2:" << std::endl

        << "movl 16(%ebp), %edx" << std::endl
        << "movl 8(%ebp), %ecx" << std::endl
        << "addl %ecx, %edx" << std::endl

        << "movl -4(%ebp), %eax" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}

void Methods::write(AssemblyFileWriter& writer) {
    writePrintString(writer.stream());
    writePrintInteger(writer.stream());
    writePrintLine(writer.stream());
	writeConcat(writer.stream());
}

#include "il/PrintStringFunction.hpp"
#include "il/PrintLineFunction.hpp"
#include "il/PrintIntegerFunction.hpp"
#include "il/ConcatFunction.hpp"

#include "il/IntermediateProgram.hpp"

void Methods::writeIL(IntermediateProgram& program){
    program.addInstruction(std::shared_ptr<Instruction>(new PrintStringFunction()));
    program.addInstruction(std::shared_ptr<Instruction>(new PrintIntegerFunction()));
    program.addInstruction(std::shared_ptr<Instruction>(new PrintLineFunction()));
    program.addInstruction(std::shared_ptr<Instruction>(new ConcatFunction()));
}
