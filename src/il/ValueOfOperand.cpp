//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/ValueOfOperand.hpp"

using namespace eddic;

ValueOfOperand::ValueOfOperand(std::string l) : Operand(OperandType::GLOBAL), location(l) {}

std::string ValueOfOperand::getValue(){
    return "(" + location + ")";
}
