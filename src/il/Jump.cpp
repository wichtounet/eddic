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
    }

    //TODO Complete the switch
}
