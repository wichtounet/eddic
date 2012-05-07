//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PROGRAM_H
#define LTAC_PROGRAM_H

#include "tac/Program.hpp"
#include "ltac/Function.hpp"

namespace eddic {

namespace ltac {

typedef tac::Program<ltac::Function> Program;

} //end of ltac

} //end of eddic

#endif
