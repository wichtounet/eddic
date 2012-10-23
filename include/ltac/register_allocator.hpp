//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_REGISTER_ALLOCATOR_H
#define LTAC_REGISTER_ALLOCATOR_H

#include <memory>

#include "Platform.hpp"

#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

void register_allocation(std::shared_ptr<mtac::Program> program, Platform platform);

} //end of mtac

} //end of eddic

#endif
