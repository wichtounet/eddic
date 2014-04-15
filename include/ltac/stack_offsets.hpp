//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_STACK_OFFSETS_H
#define LTAC_STACK_OFFSETS_H

#include "Platform.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

void fix_stack_offsets(mtac::Program& program, Platform platform);

} //end of ltac

} //end of eddic

#endif
