//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Division.hpp"

#include "Variable.hpp"

#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "il/IntermediateProgram.hpp"

using namespace eddic;

int Division::compute(int left, int right) {
    return left / right;
}

std::shared_ptr<Operand> performDivision(std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs, IntermediateProgram& program){
    std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
    std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");

    lhs->assignTo(registerA, program);
    rhs->assignTo(registerB, program);

    program.addInstruction(program.factory().createMath(Operation::DIV, registerA, registerB));

    return registerA;
}

void Division::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    program.addInstruction(program.factory().createMove(performDivision(lhs, rhs, program), operand));
}

void Division::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    assignTo(variable->toIntegerOperand(), program);
}

void Division::push(IntermediateProgram& program){
    program.addInstruction(program.factory().createPush(performDivision(lhs, rhs, program)));
}
