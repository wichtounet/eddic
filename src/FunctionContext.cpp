//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "ast/GetConstantValue.hpp"

using namespace eddic;

FunctionContext::FunctionContext(std::shared_ptr<Context> parent) : 
    Context(parent), currentPosition(::INT->size()), currentParameter(2 * ::INT->size()) {}

int FunctionContext::size() const {
    return currentPosition - ::INT->size();
}

std::shared_ptr<Variable> FunctionContext::newParameter(const std::string& variable, std::shared_ptr<const Type> type){
    Position position(PositionType::PARAMETER, currentParameter + (type->size() - ::INT->size()));

    currentParameter += type->size();

    return std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> FunctionContext::newVariable(const std::string& variable, std::shared_ptr<const Type> type){
    Position position(PositionType::STACK, currentPosition);

    currentPosition += type->size();

    return std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type){
    return variables[variable] = newVariable(variable, type);
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type, ast::Value& value){
    assert(type->is_const());

    Position position(PositionType::CONST);

    auto val = visit(ast::GetConstantValue(), value);
    return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
}

std::shared_ptr<Variable> FunctionContext::addParameter(const std::string& parameter, std::shared_ptr<const Type> type){
    return variables[parameter] = newParameter(parameter, type);
}

std::shared_ptr<Variable> FunctionContext::newTemporary(){
    Position position(PositionType::TEMPORARY);

    std::string name = "ti_" + toString(temporary++);
    return variables[name] = std::make_shared<Variable>(name, INT, position); 
}

std::shared_ptr<Variable> FunctionContext::newFloatTemporary(){
    Position position(PositionType::TEMPORARY);

    std::string name = "tf_" + toString(temporary++);
    return variables[name] = std::make_shared<Variable>(name, FLOAT, position); 
}

void FunctionContext::storeTemporary(std::shared_ptr<Variable> temp){
    Position position(PositionType::STACK, currentPosition);

    currentPosition += temp->type()->size();
   
    temp->setPosition(position); 
}
