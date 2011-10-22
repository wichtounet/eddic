//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/GlobalOperand.hpp"

#include "Utils.hpp"

using namespace eddic;

GlobalOperand::GlobalOperand(std::string label, int offset) : Operand(OperandType::GLOBAL), m_label(label), m_offset(offset) {}

std::string GlobalOperand::getValue(){
    if(m_offset > 0){
        return m_label + "+" + toString(m_offset);
    }

    return m_label;
}
