//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_BASIC_BLOCK_H
#define MTAC_BASIC_BLOCK_H

#include <boost/variant/variant.hpp>

#include "mtac/Statement.hpp"

namespace eddic {

class Context;

namespace mtac {

class BasicBlock {
    public:
        int index;
        std::string label;

        std::vector<mtac::Statement> statements;
        std::shared_ptr<FunctionContext> context;

        BasicBlock(int index);

        void add(mtac::Statement statement);
};

std::ostream& operator<<(std::ostream& stream, BasicBlock& basic_block);

} //end of mtac

} //end of eddic

#endif
