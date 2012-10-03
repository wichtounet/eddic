//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Function.hpp"

using namespace eddic;

ltac::BasicBlock::BasicBlock(int i) : index(i) {}

std::ostream& ltac::operator<<(std::ostream& stream, ltac::BasicBlock& block){
    return stream << "B" << block.index;
}
