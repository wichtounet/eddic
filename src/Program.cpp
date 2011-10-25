//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Program.hpp"
#include "Function.hpp"
#include "Context.hpp"

#include "il/IntermediateProgram.hpp"

using namespace eddic;

Program::Program(std::shared_ptr<Context> context, const Tok& token) : ParseNode(context, token) {}

void Program::addFunction(std::shared_ptr<Function> function){
    functions[function->mangledName()] = function;

    addLast(function);
}

bool Program::exists(const std::string& function){
    return functions.find(function) != functions.end();
}

void Program::writeIL(IntermediateProgram& program){
    ParseNode::writeIL(program);

    //Write the global variables
    context()->writeIL(program);
}
