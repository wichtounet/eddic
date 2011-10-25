//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Integer.hpp"

#include "Variable.hpp"

#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "il/IntermediateProgram.hpp"

using namespace eddic;
        
Integer::Integer(std::shared_ptr<Context> context, const Tok token, int value) : Value(context, token), m_value(value) {
    m_type = Type::INT;
}

bool Integer::isConstant() {
    return true;
}

bool Integer::isImmediate() {
    return true;
}

int Integer::getIntValue() {
    return m_value;
} 

void Integer::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    assignTo(variable->toIntegerOperand(), program);
}

void Integer::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    program.addInstruction(
        program.factory().createMove(
           createImmediateOperand(m_value),
           operand
        )
    ); 
}

void Integer::push(IntermediateProgram& program){
    program.addInstruction(
        program.factory().createPush(
            createImmediateOperand(m_value)
        )
    );
}
