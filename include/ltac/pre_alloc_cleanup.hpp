//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PRE_ALLOC_CLEANUP_H
#define LTAC_PRE_ALLOC_CLEANUP_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

void pre_alloc_cleanup(std::shared_ptr<mtac::Program> program);

} //end of ltac

} //end of eddic

#endif
