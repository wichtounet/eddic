//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_GOTO_H
#define MTAC_GOTO_H

#include "tac/Jump.hpp"

namespace eddic {

namespace mtac {

class BasicBlock;

enum class JumpType : unsigned int {
    ALWAYS
};

typedef tac::Jump<JumpType, BasicBlock> Goto;

} //end of mtac

} //end of eddic

#endif
