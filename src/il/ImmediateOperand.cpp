//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/ImmediateOperand.hpp"

#include "Utils.hpp"

using namespace eddic;

ImmediateOperand::ImmediateOperand(int value) : Operand(OperandType::IMMEDIATE), string(false), m_int(value) {}
ImmediateOperand::ImmediateOperand(std::string value) : Operand(OperandType::IMMEDIATE), string(true), m_string(value) {}

std::string ImmediateOperand::getValue(){
    return "$" + (string ? m_string : toString(m_int));
}
