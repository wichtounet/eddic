//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Function.hpp"

using namespace eddic;

tac::Function::Function(std::shared_ptr<FunctionContext> c) : context(c) {}

std::shared_ptr<tac::BasicBlock> tac::Function::currentBlock(){
    if(blocks.empty()){
        return newBlock();
    }

    return blocks.back();
}

std::shared_ptr<tac::BasicBlock> tac::Function::newBlock(){
    blocks.push_back(std::make_shared<BasicBlock>());
    return blocks.back();
}   
