//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/ValueOfOperand.hpp"
#include "Utils.hpp"

using namespace eddic;

//TODO It's not stack, it's memory
ValueOfOperand::ValueOfOperand(std::string l, int o) : Operand(OperandType::STACK), location(l), offset(o) {}

std::string ValueOfOperand::getValue(){
    if(offset > 0){
        return toString(offset) + "(" + location + ")";
    }

    return "(" + location + ")";
}
