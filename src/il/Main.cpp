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

void Main::write(AssemblyFileWriter& writer){
    writer.stream() << ".text" << std::endl
                    << ".globl main" << std::endl
                    << "\t.type main, @function" << std::endl;
}
