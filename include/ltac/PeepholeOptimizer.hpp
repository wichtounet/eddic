//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_PEEPHOLE_OPTIMIZER_H
#define LTAC_PEEPHOLE_OPTIMIZER_H

#include "Platform.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

void optimize(mtac::Program& program, Platform platform);

} //end of ltac

} //end of eddic

#endif
