//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_STACK_OFFSETS_H
#define LTAC_STACK_OFFSETS_H

#include "Platform.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

void fix_stack_offsets(mtac::Program& mtac_program, Platform platform);

} //end of ltac

} //end of eddic

#endif
