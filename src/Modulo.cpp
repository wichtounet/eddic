//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Modulo.hpp"

#include "Variable.hpp"

#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "il/IntermediateProgram.hpp"

using namespace eddic;
        
Modulo::Modulo(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

int Modulo::compute(int left, int right) {
    return left % right;
}

std::shared_ptr<Operand> performModulo(std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs, IntermediateProgram& program){
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
    
    program.addInstruction(program.factory().createMath(Operation::MOD, registerA, registerB));

    return registerB;
}

void Modulo::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    program.addInstruction(program.factory().createMove(performModulo(lhs, rhs, program), operand));
}

void Modulo::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    assignTo(variable->toIntegerOperand(), program);
}

void Modulo::push(IntermediateProgram& program){
    program.addInstruction(program.factory().createPush(performModulo(lhs, rhs, program)));
}
