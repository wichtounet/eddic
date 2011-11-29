//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_QUADRUPLE_H
#define TAC_QUADRUPLE_H

#include <vector>

namespace eddic {

class Variable;

namespace tac {

struct Quadruple {
    std::shared_ptr<Variable> result;
    Argument arg1;
    Argument arg2;
    Operator op;
};

} //end of tac

} //end of eddic

#endif
