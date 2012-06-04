//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Instruction.hpp"

using namespace eddic;
    
//Default constructor
ltac::Instruction::Instruction(){
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op) : op(op) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1) : op(op), arg1(arg1) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2) : op(op), arg1(arg1), arg2(arg2) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, ltac::Argument arg3) : op(op), arg1(arg1), arg2(arg2), arg3(arg3) {
    //Nothing to init
}
