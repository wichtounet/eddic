//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionTable.hpp"

using namespace eddic;

void FunctionTable::addFunction(std::shared_ptr<FunctionSignature> function){
    functions[function->mangledName] = function;
}

bool FunctionTable::exists(const std::string& function){
    return functions.find(function) != functions.end();
}
