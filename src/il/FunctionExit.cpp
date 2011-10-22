//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/FunctionExit.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

FunctionExit::FunctionExit(int size) : m_size(size) {}

void FunctionExit::write(AssemblyFileWriter& writer){
    //Only if necessary, deallocates size on the stack for the local variables
    if(m_size > 0){
        writer.stream() << "addl $" << m_size << " , %esp" << std::endl;
    }
    
    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}
