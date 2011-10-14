//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Math.hpp"
#include "il/Operand.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

Math::Math(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) : m_operation(operation), m_lhs(lhs), m_rhs(rhs) {}

void Math::write(AssemblyFileWriter& writer){
    //TODO Improve ?
    switch(m_operation){
        case Operation::ADD: 
            writer.stream() << "addl " << m_lhs->getValue() << ", " << m_rhs->getValue() << std::endl;

            break;
        case Operation::SUB:
            writer.stream() << "subl " << m_lhs->getValue() << ", " << m_rhs->getValue() << std::endl;

            break;
        case Operation::DIV:
            writer.stream() << "divl " << m_lhs->getValue() << ", " << m_rhs->getValue() << std::endl;

            break;
        case Operation::MUL:
            writer.stream() << "mull " << m_lhs->getValue() << ", " << m_rhs->getValue() << std::endl;

            break;
        case Operation::MOD:
            writer.stream() << "modl " << m_lhs->getValue() << ", " << m_rhs->getValue() << std::endl;

            break;
    }
}
