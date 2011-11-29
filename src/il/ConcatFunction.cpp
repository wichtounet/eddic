//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/ConcatFunction.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void ConcatFunction::write(AssemblyFileWriter& writer) const {
    writer.stream() << std::endl;
    writer.stream() << "concat:" << std::endl
        << "pushl %ebp" << std::endl
        << "movl %esp, %ebp" << std::endl

        //Save registers
        << "pushl %ebx" << std::endl
        << "pushl %ecx" << std::endl

        << "movl 16(%ebp), %edx" << std::endl
        << "movl 8(%ebp), %ecx" << std::endl
        << "addl %ecx, %edx" << std::endl

        << "pushl %edx" << std::endl
        << "call eddi_alloc" << std::endl
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

        //Restore registers
        << "popl %ecx" << std::endl
        << "popl %ebx" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}
