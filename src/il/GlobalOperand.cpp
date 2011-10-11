//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/GlobalOperand.hpp"

using namespace eddic;

GlobalOperand::GlobalOperand(std::string label) : Operand(OperandType::GLOBAL), m_label(label) {}

std::string GlobalOperand::getValue(){
    return m_label;
}
