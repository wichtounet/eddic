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

//Used to avoid moving one register into the same
void move(const std::string& source, const std::string& destination, AssemblyFileWriter& writer){
    if(source != destination){
        writer.stream() << "movl " << source << ", " << destination << std::endl;
    }
}

//TODO Improve before by putting directly the operand in the operation
void Math::write(AssemblyFileWriter& writer){
    switch(m_operation){
        case Operation::ADD: 
            writer.stream() << "addl " << m_lhs->getValue() << ", " << m_rhs->getValue() << std::endl;

            break;
        case Operation::SUB:
            writer.stream() << "subl " << m_rhs->getValue() << ", " << m_lhs->getValue() << std::endl;

            break;
        case Operation::DIV:
            move(m_lhs->getValue(), "%eax", writer);
            move(m_rhs->getValue(), "%ebx", writer);
            writer.stream() << "movl $0, %edx" << std::endl;
            writer.stream() << "divl %ebx" << std::endl;
            writer.stream() << "movl %eax, " << m_rhs->getValue() << std::endl;
            move("%eax", m_rhs->getValue(), writer);

            break;
        case Operation::MUL:
            move(m_lhs->getValue(), "%eax", writer);
            move(m_rhs->getValue(), "%ebx", writer);
            writer.stream() << "mull %ebx" << std::endl;
            move("%eax", m_rhs->getValue(), writer);

            break;
        case Operation::MOD:
            move(m_lhs->getValue(), "%eax", writer);
            move(m_rhs->getValue(), "%ebx", writer);
            writer.stream() << "movl $0, %edx" << std::endl;
            writer.stream() << "divl %ebx" << std::endl;
            move("%edx", m_rhs->getValue(), writer);

            break;
    }
}
