//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_FUNCTION_H
#define TAC_FUNCTION_H

#include <memory>
#include <vector>

namespace eddic {

struct FunctionContext;

namespace tac {

struct Statement;

struct Fnction {
    std::shared_ptr<FunctionContext> context;

    std::vector<Statement> statements;
};

} //end of tac

} //end of eddic

#endif
