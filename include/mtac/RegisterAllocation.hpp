//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_REGISTER_ALLOCATION_H
#define MTAC_REGISTER_ALLOCATION_H

#include "Platform.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {
    
void register_param_allocation(mtac::Program& program, Platform platform);

} //end of mtac

} //end of eddic

#endif
