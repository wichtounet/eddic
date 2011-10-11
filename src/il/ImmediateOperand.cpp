//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/ImmediateOperand.hpp"

#include "Utils.hpp"

using namespace eddic;

ImmediateOperand::ImmediateOperand(int value) : Operand(OperandType::IMMEDIATE), m_value(value) {}

std::string ImmediateOperand::getValue(){
    return "$" + toString(m_value);
}
