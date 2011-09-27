//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "AssemblyFileWriter.hpp"
#include "Functions.hpp"
#include "Context.hpp"

using namespace eddic;

using std::endl;

void MainDeclaration::write(AssemblyFileWriter& writer){
    writer.stream() << ".text" << endl
                    << ".globl main" << endl
                    << "\t.type main, @function" << endl;
}

void FunctionCall::checkFunctions(Program& program){
    if(!program.exists(m_function)){
        throw CompilerException("The function \"" + m_function + "\"does not exists");
    }
}

void Function::addParameter(std::string name, Type type){
    Parameter param(name, type, m_currentPosition);
    m_parameters.push_back(param);

    if(type == INT){
        m_currentPosition += 4;
    } else {
        m_currentPosition += 8;
    }
}

void Function::write(AssemblyFileWriter& writer){
    writer.stream() << endl << m_name << ":" << endl;
    
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    context()->write(writer);

    ParseNode::write(writer);

    context()->release(writer);

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void FunctionCall::write(AssemblyFileWriter& writer){
    std::vector<Value*>::const_iterator it = m_values.begin();
    std::vector<Value*>::const_iterator end = m_values.end();

    for( ; it != end; ++it){
        (*it)->write(writer);
    }

    writer.stream() << "call " << m_function << endl;
}
