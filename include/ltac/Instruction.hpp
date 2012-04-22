//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_QUADRUPLE_H
#define LTAC_QUADRUPLE_H

#include <memory>

#include "tac/Quadruple.hpp"

#include "ltac/Operator.hpp"
#include "ltac/Argument.hpp"

namespace eddic {

namespace ltac {

typedef tac::Quadruple<ltac::Argument, ltac::Argument, ltac::Operator> Instruction;

} //end of ltac

} //end of eddic

#endif
