//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Swap.hpp"

#include "AssemblyFileWriter.hpp"
#include "Context.hpp"
#include "Value.hpp"
#include "Variable.hpp"

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

void Swap::write(AssemblyFileWriter& writer) {
    switch (m_type) {
        case Type::INT:
            m_lhs_var->moveToRegister(writer, "%eax"); 
            m_rhs_var->moveToRegister(writer, "%ebx"); 
            
            m_lhs_var->moveFromRegister(writer, "%ebx"); 
            m_rhs_var->moveFromRegister(writer, "%eax"); 

            break;
        case Type::STRING:
            m_lhs_var->moveToRegister(writer, "%eax", "%ebx"); 
            m_rhs_var->moveToRegister(writer, "%ecx", "%edx"); 
            
            m_lhs_var->moveFromRegister(writer, "%ecx", "%edx"); 
            m_rhs_var->moveFromRegister(writer, "%eax", "%ebx"); 

            break;
        default:
            throw CompilerException("Variable of invalid type");
    }
}
