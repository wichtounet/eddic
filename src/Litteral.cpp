//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cassert>

#include "Litteral.hpp"

#include "StringPool.hpp"
#include "AssemblyFileWriter.hpp"
#include "Variable.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"

using namespace eddic;

void Litteral::checkStrings(StringPool& pool) {
    m_label = pool.label(m_litteral);
}

void Litteral::write(AssemblyFileWriter& writer) {
    writer.stream() << "pushl $" << getStringLabel() << std::endl;
    writer.stream() << "pushl $" << getStringSize() << std::endl;
}

bool Litteral::isConstant() {
    return true;
}

std::string Litteral::getStringValue() {
    return m_litteral;
}  

std::string Litteral::getStringLabel(){
    return m_label;
}

int Litteral::getStringSize(){
    return m_litteral.size() - 2;
}

void Litteral::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> operands = variable->toStringOperand();

    program.addInstruction(
        program.factory().createMove(
            createGlobalOperand(getStringLabel()),
            operands.first
        )
    );
    
    program.addInstruction(
        program.factory().createMove(
            createImmediateOperand(getStringSize()),
            operands.second
        )
    );
}

void Litteral::assignTo(std::shared_ptr<Operand>, IntermediateProgram&){
    assert(false); //Cannot assign a string to a single operand
}

void Litteral::push(IntermediateProgram& program){
    //TODO Verify that it is corresponding to pushl $label
    program.addInstruction(
        program.factory().createPush(
            createGlobalOperand(getStringLabel())
        )
    );
    
    program.addInstruction(
        program.factory().createPush(
            createImmediateOperand(getStringSize())
        )
    );
}
