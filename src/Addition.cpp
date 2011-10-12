//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Addition.hpp"
#include "AssemblyFileWriter.hpp"
#include "Options.hpp"
#include "Integer.hpp"
#include "Value.hpp"
#include "Variable.hpp"

#include "il/Operand.hpp"
#include "il/Operands.hpp"
#include "il/IntermediateProgram.hpp"

using std::string;

using namespace eddic;
        
Addition::Addition(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

Type Addition::checkTypes(Type left, Type right) {
    if (left != right) {
        throw CompilerException("Can only add two values of the same type", token());
    }

    return left;
}

void Addition::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    if (lhs->type() == Type::INT) {
        writer.stream() << "movl (%esp), %eax" << std::endl;
        writer.stream() << "movl 4(%esp), %ecx" << std::endl;
        writer.stream() << "addl %ecx, %eax" << std::endl;
        writer.stream() << "addl $8, %esp" << std::endl;
        writer.stream() << "pushl %eax" << std::endl;
    } else {
        writer.stream() << "call concat" << std::endl;
        writer.stream() << "addl $16, %esp" << std::endl;

        writer.stream() << "pushl %eax" << std::endl;
        writer.stream() << "pushl %edx" << std::endl;
    }
}

void Addition::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    //TODO 
    //Warning string
}

void Addition::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    if(lhs->type() == Type::INT){
        std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
        std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");

        lhs->assignTo(registerA, program);
        rhs->assignTo(registerB, program);

        program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

        program.addInstruction(program.factory().createMove(registerB, variable->toIntegerOperand()));
    } else {
        //TODO PUSH a, b

        program.addInstruction(program.factory().createCall("concat"));

        //TODO add 16 esp

        //TODO PUSH eax, edx
    }
}

int Addition::compute(int left, int right) {
    return left + right;
}

string Addition::compute(const std::string& left, const std::string& right) {
    return left + right;
}

void Addition::optimize() {
    if (isConstant()) {
        if (type() == Type::INT) {
            if (Options::isSet(BooleanOption::OPTIMIZE_INTEGERS)) {
                std::shared_ptr<Value> value(new Integer(context(), lhs->token(), getIntValue()));

                parent.lock()->replace(shared_from_this(), value);
            }
        } else if (type() == Type::STRING) {
            if (Options::isSet(BooleanOption::OPTIMIZE_STRINGS)) {
                //No optimization at this time
            }
        }
    }

    lhs->optimize();
    rhs->optimize();
}
