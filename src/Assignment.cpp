//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Assignment.hpp"

#include "Context.hpp"
#include "Value.hpp"
#include "Variable.hpp"

using namespace eddic;

Assignment::Assignment(std::shared_ptr<Context> context, const Tok token, const std::string& variable, std::shared_ptr<Value> v) : VariableOperation(context, token, variable, v) {}

void Assignment::checkVariables() {
    if (!context()->exists(m_variable)) {
        throw CompilerException("Variable has not  been declared", token());
    }

    m_var = context()->getVariable(m_variable);

    value->checkVariables();

    if (value->type() != m_var->type()) {
        throw CompilerException("Incompatible type", token());
    }
}
