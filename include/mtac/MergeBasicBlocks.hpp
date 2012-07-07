//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_MERGE_BASIC_BLOCKS_H
#define MTAC_MERGE_BASIC_BLOCKS_H

#include <memory>

#include "mtac/Function.hpp"

namespace eddic {

namespace mtac {

bool merge_basic_blocks(std::shared_ptr<mtac::Function> function);

} //end of mtac

} //end of eddic

#endif
