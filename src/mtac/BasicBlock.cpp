//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Function.hpp"

using namespace eddic;

tac::BasicBlock::BasicBlock(unsigned int i) : index(i), label("") {}

void tac::BasicBlock::add(tac::Statement statement){
    statements.push_back(statement);
}
