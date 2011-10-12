//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VariableValue.hpp"

#include "AssemblyFileWriter.hpp"
#include "Context.hpp"
#include "Value.hpp"
#include "Variable.hpp"

#include "il/IntermediateProgram.hpp"

using namespace eddic;

VariableValue::VariableValue(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable) : Value(context, token), m_variable(variable) {}
VariableValue::VariableValue(std::shared_ptr<Variable> var) : Value(NULL, NULL), m_var(var) {}

void VariableValue::checkVariables() {
    if (!context()->exists(m_variable)) {
        throw CompilerException("Variable has not been declared", token());
    }

    m_var = context()->getVariable(m_variable);
    m_type = m_var->type();
}

void VariableValue::write(AssemblyFileWriter& writer) {
    m_var->pushToStack(writer);
}

bool VariableValue::isConstant() {
    return false;
}

void VariableValue::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    //TODO Implement
}

void VariableValue::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    if(m_var->type() == Type::INT){
        program.addInstruction(program.factory().createMove(m_var->toIntegerOperand(), operand));
    } else {
        //TODO Arf
    }
}
