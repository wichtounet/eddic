//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/StackOperand.hpp"

#include "Utils.hpp"

using namespace eddic;

StackOperand::StackOperand(int offset) : Operand(OperandType::STACK), m_offset(offset) {}

std::string StackOperand::getValue(){
    return toString(m_offset) + "(%esp)";
}
