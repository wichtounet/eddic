//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Program.hpp"
#include "AssemblyFileWriter.hpp"
#include "Function.hpp"
#include "Context.hpp"

using namespace eddic;

Program::Program(std::shared_ptr<Context> context) : ParseNode(context) {}

void Program::addFunction(std::shared_ptr<Function> function){
    functions[function->mangledName()] = function;

    addLast(function);
}

bool Program::exists(const std::string& function){
    return functions.find(function) != functions.end();
}

void Program::write(AssemblyFileWriter& writer){
    ParseNode::write(writer);

    //Write the global variables
    context()->write(writer);
}
