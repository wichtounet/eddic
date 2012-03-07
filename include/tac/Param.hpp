//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_PARAM_H
#define TAC_PARAM_H

#include <memory>

#include "tac/Argument.hpp"

namespace eddic {

class Variable;

namespace tac {

struct Param {
    tac::Argument arg;
    std::shared_ptr<Variable> param;

    Param(const Param& rhs) = delete;
    Param& operator=(const Param& rhs) = delete;

    Param();
    Param(tac::Argument arg1);
    Param(tac::Argument arg1, std::shared_ptr<Variable> param);
};

} //end of tac

} //end of eddic

#endif
