//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalDeclaration.hpp"

#include "CompilerException.hpp"
#include "Value.hpp"
#include "Context.hpp"

using namespace eddic;

void GlobalDeclaration::checkVariables() {
    if (context()->exists(m_variable)) {
        throw CompilerException("Variable has already been declared", token());
    }
    
    value->checkVariables();

    if(!value->isConstant()){
        throw CompilerException("The value must be constant", token());
    }

    m_var = m_globalContext->addVariable(m_variable, m_type, value);

    if (value->type() != m_type) {
        throw CompilerException("Incompatible type", token());
    }
}

void GlobalDeclaration::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}
