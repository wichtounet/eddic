//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_QUADRUPLE_H
#define TAC_QUADRUPLE_H

#include <boost/optional.hpp>

#include <vector>

#include "tac/Operator.hpp"
#include "tac/Argument.hpp"

namespace eddic {

class Variable;

namespace tac {

struct Quadruple {
    std::shared_ptr<Variable> result;
    tac::Argument arg1;
    boost::optional<tac::Argument> arg2;
    boost::optional<tac::Operator> op;
    
    bool liveVariable1;
    bool liveVariable2;

    Quadruple();
    Quadruple(std::shared_ptr<Variable> result, tac::Argument arg);
    Quadruple(std::shared_ptr<Variable> result, tac::Argument arg1, Operator op, tac::Argument arg2);
};

} //end of tac

} //end of eddic

#endif
