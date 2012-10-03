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

std::string ltac::Function::getName() const {
    return name;
}
        
void ltac::Function::add(ltac::Statement statement){
    assert(current_bb());

    _basic_blocks.back()->statements.push_back(statement);
}


ltac::Function::BlockPtr ltac::Function::current_bb(){
    return _basic_blocks.back();
}

ltac::Function::BlockPtr ltac::Function::new_bb(){
    _basic_blocks.push_back(std::make_shared<ltac::BasicBlock>(_basic_blocks.size() + 1));
    return _basic_blocks.back();
}

ltac::Function::BlockList& ltac::Function::basic_blocks(){
    return _basic_blocks;
}
