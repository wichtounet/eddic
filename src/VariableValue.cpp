//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "VariableValue.hpp"
#include "CompilerException.hpp"
#include "Context.hpp"
#include "Value.hpp"
#include "Variable.hpp"

#include "il/IntermediateProgram.hpp"

using namespace eddic;

VariableValue::VariableValue(std::shared_ptr<Context> context, const Tok& token, const std::string& variable) : Value(context, token), m_variable(variable) {}
VariableValue::VariableValue(std::shared_ptr<Context> context, const Tok& token, std::shared_ptr<Variable> var) : Value(context, token), m_var(var) {}

void VariableValue::checkVariables() {
    if (!context()->exists(m_variable)) {
        throw CompilerException("Variable has not been declared", token());
    }

    m_var = context()->getVariable(m_variable);
    m_type = m_var->type();
}

bool VariableValue::isConstant() {
    return false;
}

void VariableValue::assignTo(std::shared_ptr<Variable> variable, IntermediateProgram& program){
    if(m_var->type() == Type::INT){
        program.addInstruction(program.factory().createMove(m_var->toIntegerOperand(), variable->toIntegerOperand()));
    } else {
        program.addInstruction(program.factory().createMove(m_var->toStringOperand().first, variable->toStringOperand().first));
        program.addInstruction(program.factory().createMove(m_var->toStringOperand().second, variable->toStringOperand().second));
    }
}

void VariableValue::assignTo(std::shared_ptr<Operand> operand, IntermediateProgram& program){
    if(m_var->type() == Type::INT){
        program.addInstruction(program.factory().createMove(m_var->toIntegerOperand(), operand));
    } else {
        assert(false); //Cannot assign a string to a single operand
    }
}

void VariableValue::push(IntermediateProgram& program){
    if(m_var->type() == Type::INT){
        program.addInstruction(
            program.factory().createPush(
                m_var->toIntegerOperand()
            )
        );
    } else {
        program.addInstruction(
            program.factory().createPush(
                m_var->toStringOperand().first
            )
        );
        
        program.addInstruction(
            program.factory().createPush(
                m_var->toStringOperand().second
            )
        );
    }
}
