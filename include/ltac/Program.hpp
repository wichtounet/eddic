//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PROGRAM_H
#define LTAC_PROGRAM_H

#include "ltac/Function.hpp"

namespace eddic {

struct GlobalContext;

namespace ltac {

struct Program {
    std::shared_ptr<GlobalContext> context;

    std::vector<std::shared_ptr<Function>> functions;
};

} //end of ltac

} //end of eddic

#endif
