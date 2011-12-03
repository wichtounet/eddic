//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Function.hpp"

using namespace eddic;

tac::BasicBlock& tac::Function::currentBlock(){
    if(blocks.empty()){
        return newBlock();
    }

    return blocks.back();
}

tac::BasicBlock& tac::Function::newBlock(){
    tac::BasicBlock block;
    blocks.push_back(block);
    return blocks.back();
}   
