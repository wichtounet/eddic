//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/ImmediateOperand.hpp"

#include "Utils.hpp"

using namespace eddic;

ImmediateIntegerOperand::ImmediateIntegerOperand(int value) : Operand(OperandType::IMMEDIATE), m_int(value) {}
ImmediateStringOperand::ImmediateStringOperand(const std::string& value) : Operand(OperandType::IMMEDIATE), m_string(value) {}

std::string ImmediateIntegerOperand::getValue() const {
    return "$" + toString(m_int);
}

std::string ImmediateStringOperand::getValue() const {
    return "$" + m_string;
}
