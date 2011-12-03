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

namespace tac {

struct BasicBlock {
    std::shared_ptr<FunctionContext> context;
    
    std::vector<Statement> statements;
    
    void add(Statement statement);
};

} //end of tac

} //end of eddic

#endif
