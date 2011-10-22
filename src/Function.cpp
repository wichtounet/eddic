//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Function.hpp"
#include "Context.hpp"
#include "Parameter.hpp"
#include "mangling.hpp"

#include "il/IntermediateProgram.hpp"

using namespace eddic;

using std::endl;

Function::Function(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& name) : ParseNode(context, token), m_name(name), m_currentPosition(0) {}

const std::string& Function::name() const {
    return m_name;
}

const std::string& Function::mangledName() const {
    if(m_mangled_name.empty()){
        m_mangled_name = mangle(m_name, m_parameters);
    }

    return m_mangled_name;
}
		
void Function::addParameter(const std::string& name, Type type){
    std::shared_ptr<Parameter> param(new Parameter(name, type, m_currentPosition));
    m_parameters.push_back(param);

    m_currentPosition += size(type);
}

void Function::writeIL(IntermediateProgram& program){
    program.addInstruction(program.factory().createFunctionDeclaration(mangledName(), context()->size()));

    ParseNode::writeIL(program);

    program.addInstruction(program.factory().createFunctionExit(context()->size()));
}
