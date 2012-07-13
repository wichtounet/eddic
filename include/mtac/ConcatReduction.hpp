//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONCAT_REDUCTION_H
#define MTAC_CONCAT_REDUCTION_H

#include <memory>

#include "mtac/Function.hpp"

namespace eddic {

class StringPool;

namespace mtac {

bool optimize_concat(std::shared_ptr<mtac::Function> function, std::shared_ptr<StringPool> pool);

} //end of mtac

} //end of eddic

#endif
