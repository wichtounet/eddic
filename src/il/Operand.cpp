//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Operand.hpp"

using namespace eddic;

Operand::Operand(OperandType type) : m_type(type) {}

bool Operand::isImmediate(){
    return m_type == OperandType::IMMEDIATE;
}

bool Operand::isRegister(){
    return m_type == OperandType::REGISTER;
}

bool Operand::isStack(){
    return m_type == OperandType::STACK;
}

bool Operand::isGlobal(){
    return m_type == OperandType::GLOBAL;
}
