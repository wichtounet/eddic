//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_BASIC_BLOCK_H
#define LTAC_BASIC_BLOCK_H

#include "variant.hpp"

#include "ltac/Statement.hpp"

namespace eddic {

namespace ltac {

class BasicBlock {
    public:
        int index;
        std::vector<ltac::Statement> l_statements;

        BasicBlock(int index);
};

std::ostream& operator<<(std::ostream& stream, BasicBlock& basic_block);

} //end of ltac

} //end of eddic

#endif
