//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DOMINATORS_H
#define MTAC_DOMINATORS_H

#include <memory>

namespace eddic {

namespace mtac {

class Function;

void compute_dominators(std::shared_ptr<Function> function);

} //end of mtac

} //end of eddic

#endif
