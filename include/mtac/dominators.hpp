//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DOMINATORS_H
#define MTAC_DOMINATORS_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

void compute_dominators(mtac::Function& function);

} //end of mtac

} //end of eddic

#endif
