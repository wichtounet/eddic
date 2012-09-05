//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "BlockContext.hpp"
#include "Variable.hpp"
#include "FunctionContext.hpp"
#include "Position.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "ast/GetConstantValue.hpp"

using namespace eddic;

BlockContext::BlockContext(std::shared_ptr<Context> parent, std::shared_ptr<FunctionContext> functionContext, std::shared_ptr<GlobalContext> global_context) : 
    Context(parent, global_context), m_functionContext(functionContext){} 

std::shared_ptr<Variable> BlockContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type){
    return variables[variable] = m_functionContext->newVariable(variable, type);
}

std::shared_ptr<Variable> BlockContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type, ast::Value& value){
    assert(type->is_const());

    Position position(PositionType::CONST);

    auto val = visit(ast::GetConstantValue(), value);

    return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
}

std::shared_ptr<Variable> BlockContext::new_temporary(std::shared_ptr<const Type> type){
    return m_functionContext->new_temporary(type);
}

std::shared_ptr<FunctionContext> BlockContext::function(){
    return m_functionContext;
}
