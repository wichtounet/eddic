//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Print.hpp"
#include "CompilerException.hpp"

#include "Value.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"

using namespace eddic;

Print::Print(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> v) : ParseNode(context, token), value(v) {}

void Print::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}

void Print::checkVariables() {
    value->checkVariables();
}

void Print::writeIL(IntermediateProgram& program){
    value->push(program);

    switch (value->type()) {
        case Type::INT:
            program.addInstruction(program.factory().createCall("print_integer"));
            program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(4), createRegisterOperand("esp")));

            break;
        case Type::STRING:
            program.addInstruction(program.factory().createCall("print_string"));
            program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(8), createRegisterOperand("esp")));

            break;
        default:
            throw CompilerException("Variable of invalid type");
    }
}
