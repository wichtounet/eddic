//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "AssemblyFileWriter.hpp"
#include "Functions.hpp"
#include "Utils.hpp"

using namespace eddic;

using std::endl;

void MainDeclaration::write(AssemblyFileWriter& writer){
    writer.stream() << ".text" << endl
                    << ".globl main" << endl
                    << "\t.type main, @function" << endl;
}

void FunctionCall::checkFunctions(Program& program){
    if(!program.exists(m_function)){
        throw CompilerException("The function \"" + m_function + "()\" does not exists", token());
    }
}

void Function::write(AssemblyFileWriter& writer){
    writer.stream() << endl << m_name << ":" << endl;
    
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;
    
    ParseNode::write(writer);

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void FunctionCall::write(AssemblyFileWriter& writer){
    writer.stream() << "call " << m_function << endl;
}
