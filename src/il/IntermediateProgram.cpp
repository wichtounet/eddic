//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include "il/IntermediateProgram.hpp"
#include "il/Instruction.hpp"

#include "il/InstructionFactory32.hpp"

using namespace eddic;

IntermediateProgram::IntermediateProgram(){
    m_factory = std::make_shared<InstructionFactory32>();
}

const InstructionFactory& IntermediateProgram::factory(){
    return *m_factory;
}

void IntermediateProgram::addInstruction(std::shared_ptr<Instruction> instruction){
    m_instructions.push_back(instruction);
}

void IntermediateProgram::writeAsm(AssemblyFileWriter& writer){
    for_each(m_instructions.begin(), m_instructions.end(), [&](std::shared_ptr<Instruction> i){ i->write(writer); });
}
