//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Function.hpp"

using namespace eddic;

mtac::BasicBlock::BasicBlock(int i) : index(i), label("") {}

void mtac::BasicBlock::add(mtac::Statement statement){
    statements.push_back(statement);
}

mtac::BasicBlock::iterator mtac::BasicBlock::begin(){
    return statements.begin();
}

mtac::BasicBlock::iterator mtac::BasicBlock::end(){
    return statements.end();
}

std::ostream& mtac::operator<<(std::ostream& stream, std::shared_ptr<BasicBlock>& basic_block){
    if(basic_block){
        return stream << *basic_block;
    } else {
        return stream << "null_bb";
    }
}

std::ostream& mtac::operator<<(std::ostream& stream, BasicBlock& block){
    if(block.index == -1){
        return stream << "ENTRY";
    } else if(block.index == -2){
        return stream << "EXIT";
    } else {
        return stream << "B" << block.index;
    }
}

mtac::BasicBlock::iterator mtac::begin(mtac::basic_block_p block){
    return block->begin();
}

mtac::BasicBlock::iterator mtac::end(mtac::basic_block_p block){
    return block->end(); 
}
