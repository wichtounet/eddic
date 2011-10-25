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

Subtraction::Subtraction(std::shared_ptr<Context> context, const Tok token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

int Subtraction::compute(int left, int right) {
    return left - right;
}

std::shared_ptr<Operand> performSubtraction(std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs, IntermediateProgram& program){
    std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
    std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");

    if(lhs->isImmediate() && rhs->isImmediate()){
        lhs->assignTo(registerA, program);
        rhs->assignTo(registerB, program);
    } else { //TODO Certainly a better way to manage this case (if only one is immediate ? )
        lhs->push(program);
        rhs->push(program);

        program.addInstruction(program.factory().createMove(createStackOperand(4), registerA));
        program.addInstruction(program.factory().createMove(createStackOperand(0), registerB));

        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(8), createRegisterOperand("esp")));
    }
    
    program.addInstruction(program.factory().createMath(Operation::SUB, registerA, registerB));

    //Because the operands are reversed A=A-B
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
