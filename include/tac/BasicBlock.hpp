//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_BASIC_BLOCK_H
#define TAC_BASIC_BLOCK_H

#include <boost/variant/variant.hpp>

#include "tac/Statement.hpp"

namespace eddic {

class Context;

namespace tac {

typedef std::shared_ptr<tac::Statement> StatementPtr;

class BasicBlock {
    public:
        unsigned int index;

        BasicBlock(unsigned int index);

        void add(StatementPtr statement);

        std::vector<StatementPtr> statements;

    private:
        std::shared_ptr<Context> context;
};

} //end of tac

} //end of eddic

#endif
