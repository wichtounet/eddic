//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/FunctionDeclaration.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

FunctionDeclaration::FunctionDeclaration(std::string name, int size) : m_name(name), m_size(size) {}

void FunctionDeclaration::write(AssemblyFileWriter& writer){
    writer.stream() << std::endl << m_name << ":" << std::endl;
    
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    //Only if necessary, allocates size on the stack for the local variables
    if(m_size > 0){
        writer.stream() << "subl $" << m_size << " , %esp" << std::endl;
    }
}
