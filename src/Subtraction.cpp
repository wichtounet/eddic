//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Subtraction.hpp"

#include "Variable.hpp"

#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "il/IntermediateProgram.hpp"

using namespace eddic;

Subtraction::Subtraction(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

int Subtraction::compute(int left, int right) {
    return left - right;
}

std::shared_ptr<Operand> performSubtraction(std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs, IntermediateProgram& program){
    std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
    std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");

    lhs->assignTo(registerA, program);
    rhs->assignTo(registerB, program);

    program.addInstruction(program.factory().createMath(Operation::SUB, registerB, registerA));

    return registerA;
}

void Subtraction::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    program.addInstruction(program.factory().createMove(performSubtraction(lhs, rhs, program), operand));
}

void Subtraction::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    assignTo(variable->toIntegerOperand(), program);
}

void Subtraction::push(IntermediateProgram& program){
    program.addInstruction(program.factory().createPush(performSubtraction(lhs, rhs, program)));
}
