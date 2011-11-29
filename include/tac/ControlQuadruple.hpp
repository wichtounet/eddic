//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_CONTROL_QUADRUPLE_H
#define TAC_CONTROL_QUADRUPLE_H

#include <vector>

namespace eddic {

class Variable;

namespace tac {

struct ControlQuadruple {
    std::string label;
    Argument arg1;
    Argument arg2;
    BinaryOperator op;
};

} //end of tac

} //end of eddic

#endif
