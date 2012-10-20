//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PEEPHOLE_OPTIMIZER_H
#define LTAC_PEEPHOLE_OPTIMIZER_H

#include <memory>

#include "Platform.hpp"

#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

void optimize(std::shared_ptr<mtac::Program> program, Platform platform);

} //end of ltac

} //end of eddic

#endif
