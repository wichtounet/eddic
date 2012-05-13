//=======================================================================
// Copyright Baptiste Wicht 2011.
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
