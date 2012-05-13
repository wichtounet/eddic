//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Function.hpp"

using namespace eddic;

ltac::Function::Function(std::shared_ptr<FunctionContext> c, const std::string& n) : context(c), name(n) {
    //Nothing to do   
}
        
void ltac::Function::add(ltac::Statement statement){
    statements.push_back(statement);
}

std::string ltac::Function::getName() const {
    return name;
}

std::vector<ltac::Statement>& ltac::Function::getStatements(){
    return statements;
}

