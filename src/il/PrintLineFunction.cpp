//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/PrintLineFunction.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void PrintLineFunction::write(AssemblyFileWriter& writer) const {
    writer.stream() << std::endl;
    writer.stream() << "print_line:" << std::endl;
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    writer.stream() << "pushl $S1" << std::endl;
    writer.stream() << "pushl $1" << std::endl;
    writer.stream() << "call print_string" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}
