//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Jump.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

Jump::Jump(JumpCondition condition, std::string label) : m_condition(condition), m_label(label) {}

void Jump::write(AssemblyFileWriter& writer){
    switch(m_condition){
        case JumpCondition::ALWAYS:
            writer.stream() << "jmp " << m_label << std::endl;
            
            break;
        case JumpCondition::LESS:
            writer.stream() << "jl " << m_label << std::endl;
            
            break;
        case JumpCondition::GREATER:
            writer.stream() << "jg " << m_label << std::endl;
            
            break;
        case JumpCondition::EQUALS:
            writer.stream() << "je " << m_label << std::endl;
            
            break;
        case JumpCondition::NOT_EQUALS:
            writer.stream() << "jne " << m_label << std::endl;
            
            break;
        case JumpCondition::GREATER_EQUALS:
            writer.stream() << "jge " << m_label << std::endl;
            
            break;
        case JumpCondition::LESS_EQUALS:
            writer.stream() << "jle " << m_label << std::endl;
            
            break;
    }

    //TODO Complete the switch
}
