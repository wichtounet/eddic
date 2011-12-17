//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_PARAM_H
#define TAC_PARAM_H

#include <unordered_map>

#include "Argument.hpp"

namespace eddic {

namespace tac {

struct Param {
    Argument arg;
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    Param();
    Param(Argument argument);
};

} //end of tac

} //end of eddic

#endif
