//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_REGISTER_ALLOCATOR_H
#define LTAC_REGISTER_ALLOCATOR_H

#include <memory>

#include "Platform.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

void register_allocation(mtac::Program& program, Platform platform);

} //end of mtac

} //end of eddic

#endif
