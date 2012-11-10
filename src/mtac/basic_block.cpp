//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Function.hpp"

using namespace eddic;

mtac::basic_block::basic_block(int i) : index(i), label("") {}

void mtac::basic_block::add(mtac::Statement statement){
    statements.push_back(statement);
}

mtac::basic_block::iterator mtac::basic_block::begin(){
    return statements.begin();
}

mtac::basic_block::iterator mtac::basic_block::end(){
    return statements.end();
}

std::ostream& mtac::operator<<(std::ostream& stream, std::shared_ptr<basic_block>& basic_block){
    if(basic_block){
        return stream << *basic_block;
    } else {
        return stream << "null_bb";
    }
}

std::ostream& mtac::operator<<(std::ostream& stream, basic_block& block){
    if(block.index == -1){
        return stream << "ENTRY";
    } else if(block.index == -2){
        return stream << "EXIT";
    } else {
        return stream << "B" << block.index;
    }
}

mtac::basic_block::iterator mtac::begin(mtac::basic_block_p block){
    return block->begin();
}

mtac::basic_block::iterator mtac::end(mtac::basic_block_p block){
    return block->end(); 
}
    
void pretty_print(std::vector<mtac::basic_block_p> blocks, std::ostream& stream){
    if(blocks.empty()){
        stream << "{}";
    } else {
        stream << "{" << blocks[0];

        for(std::size_t i = 1; i < blocks.size(); ++i){
            stream << ", " << blocks[i];
        }

        stream << "}";
    }
}

void mtac::pretty_print(mtac::basic_block_p block, std::ostream& stream){
    std::string sep(25, '-');

    stream << sep << std::endl;
    stream << block;

    stream << " prev: " << block->prev << ", next: " << block->next << std::endl;
    stream << "successors "; ::pretty_print(block->successors, stream); std::cout << std::endl;;
    stream << "predecessors "; ::pretty_print(block->predecessors, stream); std::cout << std::endl;;

    stream << sep << std::endl;
}
