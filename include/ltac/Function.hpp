//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_FUNCTION_H
#define LTAC_FUNCTION_H

#include "tac/Function.hpp"
#include "ltac/BasicBlock.hpp"
#include "ltac/Statement.hpp"

namespace eddic {

namespace ltac {

typedef tac::Function<BasicBlock, Statement> Function;

} //end of mtac

} //end of eddic

#endif
