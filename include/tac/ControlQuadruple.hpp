//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_CONTROL_QUADRUPLE_H
#define TAC_CONTROL_QUADRUPLE_H

#include <string>
#include <memory>

#include "tac/BinaryOperator.hpp"
#include "tac/Argument.hpp"

namespace eddic {

class Variable;

namespace tac {

class BasicBlock;

struct ControlQuadruple {
    Argument arg1;
    Argument arg2;
    BinaryOperator op;
    std::string label;
    
    //Filled only in later phase replacing the label
    std::shared_ptr<BasicBlock> block;

    ControlQuadruple();
    ControlQuadruple(BinaryOperator op, const std::string& label);
    ControlQuadruple(BinaryOperator op, Argument arg1, Argument arg2, const std::string& label);
};

} //end of tac

} //end of eddic

#endif
