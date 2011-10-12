//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Integer.hpp"

#include "AssemblyFileWriter.hpp"
#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "Variable.hpp"
#include "il/IntermediateProgram.hpp"

using namespace eddic;

void Integer::write(AssemblyFileWriter& writer) {
    writer.stream() << "pushl $" << m_value << std::endl;
}

bool Integer::isConstant() {
    return true;
}

int Integer::getIntValue() {
    return m_value;
} 

//TODO Move Into variabl
std::shared_ptr<Operand> toIntegerOperand(std::shared_ptr<Variable> variable){
    if(variable->position().isStack()){
        return createBaseStackOperand(variable->position().offset());
    } else if(variable->position().isParameter()){
        return createBaseStackOperand(variable->position().offset());
    } else if(variable->position().isGlobal()){
        return createGlobalOperand(variable->position().name());
    }

    throw "ERROR";
}

void Integer::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    program.addInstruction(
        program.factory().createMove(
           createImmediateOperand(m_value),
           toIntegerOperand(variable)
        )
    ); 
}
