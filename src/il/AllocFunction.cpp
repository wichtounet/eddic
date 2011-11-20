//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/AllocFunction.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void AllocFunction::write(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "eddi_alloc:" << std::endl
        << "pushl %ebp" << std::endl
        << "movl %esp, %ebp" << std::endl

        << "movl 8(%ebp), %ecx" << std::endl
        << "movl VIeddi_remaining, %ebx" << std::endl

        << "cmpl %ebx, %ecx" << std::endl
        << "jle alloc_normal" << std::endl

        //Alloc new block
        << "movl $45, %eax" << std::endl        //45 = sys_brk
        << "movl $16384, %ebx" << std::endl     //alloc 16K
        << "int  $0x80" << std::endl

        << "movl $16384, VIeddi_remaining" << std::endl
        << "movl %eax, VIeddi_current" << std::endl

        << "jmp alloc_end" << std::endl

        << "alloc_normal:" << std::endl

        //old = current
        << "movl VIeddi_current, %eax" << std::endl

        << "alloc_end:" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}
