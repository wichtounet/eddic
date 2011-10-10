//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/IntermediateProgram.hpp"
#include "il/Instruction.hpp"

using namespace eddic;

InstructionFactory IntermediateProgram::factory(){
    return m_factory;
}

void IntermediateProgram::addInstruction(Instruction instruction){
    m_instructions.push_back(instruction);
}

void IntermediateProgram::writeAsm(AssemblyFileWriter& writer){
    //TODO
}
