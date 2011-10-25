//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Swap.hpp"

#include "CompilerException.hpp"
#include "Context.hpp"
#include "Value.hpp"
#include "Variable.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"

using namespace eddic;

Swap::Swap(std::shared_ptr<Context> context, const Tok token, const std::string& lhs, const std::string& rhs) : ParseNode(context, token), m_lhs(lhs), m_rhs(rhs) {}

void Swap::checkVariables() {
    if (m_lhs == m_rhs) {
        throw CompilerException("Cannot swap a variable with itself", token());
    }

    if (!context()->exists(m_lhs) || !context()->exists(m_rhs)) {
        throw CompilerException("Variable has not been declared", token());
    }

    m_lhs_var = context()->getVariable(m_lhs);
    m_rhs_var = context()->getVariable(m_rhs);

    if (m_lhs_var->type() != m_rhs_var->type()) {
        throw CompilerException("Incompatible type", token());
    }

    m_type = m_lhs_var->type();
}

void Swap::writeIL(IntermediateProgram& program){
    switch (m_type) {
        case Type::INT:{
            std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
            std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");
       
            program.addInstruction(program.factory().createMove(m_lhs_var->toIntegerOperand(), registerA));
            program.addInstruction(program.factory().createMove(m_rhs_var->toIntegerOperand(), registerB));

            program.addInstruction(program.factory().createMove(registerB, m_lhs_var->toIntegerOperand()));
            program.addInstruction(program.factory().createMove(registerA, m_rhs_var->toIntegerOperand()));

            break;
        }
        case Type::STRING:{
            std::shared_ptr<Operand> registerA = createRegisterOperand("eax");
            std::shared_ptr<Operand> registerB = createRegisterOperand("ebx");
            std::shared_ptr<Operand> registerC = createRegisterOperand("ecx");
            std::shared_ptr<Operand> registerD = createRegisterOperand("edx");
            
            program.addInstruction(program.factory().createMove(m_lhs_var->toStringOperand().first, registerA));
            program.addInstruction(program.factory().createMove(m_lhs_var->toStringOperand().second, registerB));
            program.addInstruction(program.factory().createMove(m_rhs_var->toStringOperand().first, registerC));
            program.addInstruction(program.factory().createMove(m_rhs_var->toStringOperand().second, registerD));
            
            program.addInstruction(program.factory().createMove(registerC, m_lhs_var->toStringOperand().first));
            program.addInstruction(program.factory().createMove(registerD, m_lhs_var->toStringOperand().second));
            program.addInstruction(program.factory().createMove(registerA, m_rhs_var->toStringOperand().first));
            program.addInstruction(program.factory().createMove(registerB, m_rhs_var->toStringOperand().second));

            break;
        }
        default:
           throw CompilerException("Variable of invalid type", token());
    }
}
