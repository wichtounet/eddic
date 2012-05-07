//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_BASIC_BLOCK_H
#define LTAC_BASIC_BLOCK_H

#include <boost/variant/variant.hpp>

#include "ltac/Statement.hpp"

namespace eddic {

class Context;

namespace ltac {

class BasicBlock {
    public:
        unsigned int index;
        std::string label;

        BasicBlock(unsigned int index);

        void add(ltac::Statement statement);

        std::vector<ltac::Statement> statements;

    private:
        std::shared_ptr<Context> context;
};

} //end of ltac

} //end of eddic

#endif
