//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Function.hpp"

using namespace eddic;

tac::Function::Function(std::shared_ptr<FunctionContext> c, const std::string& n) : context(c), name(n) {}
        
void tac::Function::add(tac::Statement statement){
    statements.push_back(statement);
}

std::shared_ptr<tac::BasicBlock> tac::Function::currentBasicBlock(){
    assert(!blocks.empty());

    return blocks.back();
}

std::shared_ptr<tac::BasicBlock> tac::Function::newBasicBlock(){
    blocks.push_back(std::make_shared<BasicBlock>());
    return blocks.back();
}   

std::string tac::Function::getName() const {
    return name;
}

std::vector<tac::Statement>& tac::Function::getStatements(){
    return statements;
}
