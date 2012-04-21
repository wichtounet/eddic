//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_BASIC_BLOCK_H
#define TAC_BASIC_BLOCK_H

#include <boost/variant/variant.hpp>

#include "mtac/Statement.hpp"

namespace eddic {

class Context;

namespace mtac {

class BasicBlock {
    public:
        unsigned int index;
        std::string label;

        BasicBlock(unsigned int index);

        void add(mtac::Statement statement);

        std::vector<mtac::Statement> statements;

    private:
        std::shared_ptr<Context> context;
};

} //end of tac

} //end of eddic

#endif
