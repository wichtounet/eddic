//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/Main.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void Main::write(AssemblyFileWriter& writer) const {
    writer.stream() << ".text" << std::endl
                    << ".globl _start" << std::endl
                    
                    << "_start:" << std::endl
                    << "call main" << std::endl
                    << "movl $1, %eax" << std::endl
                    << "xorl %ebx, %ebx" << std::endl
                    << "int $0x80" << std::endl;
}
