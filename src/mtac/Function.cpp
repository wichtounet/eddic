//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Function.hpp"

using namespace eddic;

mtac::Function::Function(std::shared_ptr<FunctionContext> c, const std::string& n) : context(c), name(n) {}
        
void mtac::Function::add(mtac::Statement statement){
    statements.push_back(statement);
}

std::shared_ptr<mtac::BasicBlock> mtac::Function::currentBasicBlock(){
    assert(!blocks.empty());

    return blocks.back();
}

std::shared_ptr<mtac::BasicBlock> mtac::Function::newBasicBlock(){
    blocks.push_back(std::make_shared<BasicBlock>(blocks.size() + 1));
    return blocks.back();
}   

std::string mtac::Function::getName() const {
    return name;
}

std::vector<mtac::Statement>& mtac::Function::getStatements(){
    return statements;
}

std::vector<std::shared_ptr<mtac::BasicBlock>>& mtac::Function::getBasicBlocks(){
    return blocks;
}
