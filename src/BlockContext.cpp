//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "BlockContext.hpp"
#include "FunctionContext.hpp"

using namespace eddic;

std::shared_ptr<Variable> BlockContext::addVariable(const std::string& variable, Type type){
    std::shared_ptr<Variable> v = m_functionContext->newVariable(variable, type);

    m_visibles[variable] = currentVariable;

    m_functionContext->storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}
