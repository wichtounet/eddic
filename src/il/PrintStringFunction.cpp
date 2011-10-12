//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/PrintStringFunction.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void PrintStringFunction::write(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "print_string:" << std::endl;
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;
    writer.stream() << "movl $0, %esi" << std::endl;

    writer.stream() << "movl $4, %eax" << std::endl;
    writer.stream() << "movl $1, %ebx" << std::endl;
    writer.stream() << "movl 12(%ebp), %ecx" << std::endl;
    writer.stream() << "movl 8(%ebp), %edx" << std::endl;
    writer.stream() << "int $0x80" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}
