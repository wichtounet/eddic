//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Subtraction.hpp"
#include "AssemblyFileWriter.hpp"

#include "Variable.hpp"

#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "il/IntermediateProgram.hpp"

using namespace eddic;

Subtraction::Subtraction(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

void Subtraction::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    writer.stream() << "movl (%esp), %ecx" << std::endl;
    writer.stream() << "movl 4(%esp), %eax" << std::endl;
    writer.stream() << "subl %ecx, %eax" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
    writer.stream() << "pushl %eax" << std::endl;
}

int Subtraction::compute(int left, int right) {
    return left - right;
}

void Subtraction::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
    std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");

    lhs->assignTo(registerA, program);
    rhs->assignTo(registerB, program);

    program.addInstruction(program.factory().createMath(Operation::SUB, registerB, registerA));

    program.addInstruction(program.factory().createMove(registerA, operand));
}

void Subtraction::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    assignTo(variable->toIntegerOperand(), program);
}

void Subtraction::push(IntermediateProgram& program){
    std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
    std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");

    lhs->assignTo(registerA, program);
    rhs->assignTo(registerB, program);

    program.addInstruction(program.factory().createMath(Operation::SUB, registerB, registerA));

    program.addInstruction(program.factory().createPush(registerA));
}
