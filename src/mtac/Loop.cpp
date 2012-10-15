//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Loop.hpp"
#include "mtac/BasicBlock.hpp"

using namespace eddic;

mtac::Loop::Loop(const std::set<std::shared_ptr<mtac::BasicBlock>>& blocks) : m_blocks(blocks) {
    //Nothing
}

mtac::Loop::iterator mtac::Loop::begin(){
    return m_blocks.begin();
}

mtac::Loop::iterator mtac::Loop::end(){
    return m_blocks.begin();
}

int mtac::Loop::estimate(){
    return m_estimate;
}

void mtac::Loop::set_estimate(int estimate){
    this->m_estimate = estimate;
}

mtac::Loop::iterator mtac::begin(std::shared_ptr<mtac::Loop> loop){
    return loop->end();
}

mtac::Loop::iterator mtac::end(std::shared_ptr<mtac::Loop> loop){
    return loop->begin();
}
      
std::set<std::shared_ptr<mtac::BasicBlock>>& mtac::Loop::blocks(){
    return m_blocks;
}
