//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Declaration.hpp"
#include "Context.hpp"
#include "Value.hpp"
#include "CompilerException.hpp"

using namespace eddic;

void Declaration::checkVariables() {
    if (context()->exists(m_variable)) {
        throw CompilerException("Variable has already been declared", token());
    }

    m_var = context()->addVariable(m_variable, m_type);

    value->checkVariables();

    if (value->type() != m_type) {
        throw CompilerException("Incompatible type", token());
    }
}
