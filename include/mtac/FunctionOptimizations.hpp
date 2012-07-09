//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_FUNCTION_OPTIMIZATIONS_H
#define MTAC_FUNCTION_OPTIMIZATIONS_H

#include <memory>

#include "mtac/Program.hpp"

namespace eddic {

namespace mtac {

bool remove_unused_functions(std::shared_ptr<mtac::Program> program);
bool remove_empty_functions(std::shared_ptr<mtac::Program> program);

} //end of mtac

} //end of eddic

#endif
