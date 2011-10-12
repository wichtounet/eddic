//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/PrintIntegerFunction.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void PrintIntegerFunction::write(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "print_integer:" << std::endl
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
