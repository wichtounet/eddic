//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <algorithm>

#include "AssemblyFileWriter.hpp"
#include "Functions.hpp"
#include "Context.hpp"
#include "Utils.hpp"

using namespace eddic;

using std::endl;
using std::shared_ptr;

std::string eddic::mangle(Type type){
    if(type == Type::INT){
        return "I";
    } else {
        return "S";
    }
}

void MainDeclaration::write(AssemblyFileWriter& writer){
    writer.stream() << ".text" << endl
                    << ".globl main" << endl
                    << "\t.type main, @function" << endl;
}

void FunctionCall::checkFunctions(Program& program){
    m_function_mangled = mangle(m_function, m_values);

    if(!program.exists(m_function_mangled)){
        throw CompilerException("The function \"" + m_function + "()\" does not exists", token());
    }
}

void Function::addParameter(std::string name, Type type){
    shared_ptr<Parameter> param(new Parameter(name, type, m_currentPosition));
    m_parameters.push_back(param);

    m_currentPosition += size(type);
}

void Function::write(AssemblyFileWriter& writer){
    writer.stream() << endl << mangledName() << ":" << endl;
    
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    context()->write(writer);

    ParseNode::write(writer);

    context()->release(writer);

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void FunctionCall::write(AssemblyFileWriter& writer){
    for_each(m_values.rbegin(), m_values.rend(), [&](std::shared_ptr<Value> v){ v->write(writer); });

    writer.stream() << "call " << m_function_mangled << endl;
}
