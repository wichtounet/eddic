//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/RegisterOperand.hpp"

using namespace eddic;

RegisterOperand::RegisterOperand(const std::string& reg) : Operand(OperandType::REGISTER), m_register(reg) {}

std::string RegisterOperand::getValue() const {
    return m_register;
}
