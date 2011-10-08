//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Function.hpp"
#include "Context.hpp"
#include "AssemblyFileWriter.hpp"
#include "Parameter.hpp"
#include "mangling.hpp"

using namespace eddic;

using std::endl;

Function::Function(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& name) : ParseNode(context, token), m_name(name), m_currentPosition(0) {}

std::string Function::name(){
    return m_name;
}

std::string Function::mangledName(){
    return mangle(m_name, m_parameters);
}
		
void Function::addParameter(std::string name, Type type){
    std::shared_ptr<Parameter> param(new Parameter(name, type, m_currentPosition));
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
