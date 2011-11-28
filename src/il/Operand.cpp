//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Operand.hpp"
#include "il/ValueOfOperand.hpp"

using namespace eddic;

Operand::Operand(OperandType type) : m_type(type) {}

bool Operand::isImmediate() const {
    return m_type == OperandType::IMMEDIATE;
}

bool Operand::isRegister() const {
    return m_type == OperandType::REGISTER;
}

bool Operand::isStack() const {
    return m_type == OperandType::STACK;
}

bool Operand::isGlobal() const {
    return m_type == OperandType::GLOBAL;
}
        
std::shared_ptr<Operand> Operand::valueOf(int offset) const {
    return std::make_shared<ValueOfOperand>(getValue(), offset);
}
