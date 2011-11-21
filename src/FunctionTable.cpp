//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionTable.hpp"

using namespace eddic;

FunctionTable::FunctionTable(){}

void FunctionTable::addFunction(std::shared_ptr<Function> function){
    functions[function->mangledName] = function;
}

bool FunctionTable::exists(const std::string& function){
    return functions.find(function) != functions.end();
}

void FunctionTable::addReference(const std::string& function){
    ++(functions[function]->references);
}

int FunctionTable::referenceCount(const std::string& function){
    return functions[function]->references;
}
