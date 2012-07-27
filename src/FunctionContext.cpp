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
#include "Options.hpp"

#include "ast/GetConstantValue.hpp"

using namespace eddic;

FunctionContext::FunctionContext(std::shared_ptr<Context> parent) : Context(parent){
    currentPosition = -INT->size(); 
    
    if(option_defined("fomit-frame-pointer")){
        currentParameter = INT->size();
    } else {
        currentParameter = 2 * INT->size();
    }
}

int FunctionContext::size() const {
    auto size = -currentPosition;

    if(size == -INT->size()){
        return 0;
    }

    return size;
}

std::shared_ptr<Variable> FunctionContext::newParameter(const std::string& variable, std::shared_ptr<const Type> type){
    Position position(PositionType::PARAMETER, currentParameter);

    currentParameter += type->size();

    return std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> FunctionContext::newVariable(const std::string& variable, std::shared_ptr<const Type> type){
    currentPosition -= type->size();

    Position position(PositionType::STACK, currentPosition + INT->size());
    auto var = std::make_shared<Variable>(variable, type, position);

    storage[variable] = var;

    return var;
}

FunctionContext::Variables FunctionContext::stored_variables(){
    return storage;
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type){
    return variables[variable] = newVariable(variable, type);
}

std::shared_ptr<Variable> FunctionContext::newVariable(std::shared_ptr<Variable> source){
    std::string name = "g_" + source->name() + "_" + toString(temporary++);
    
    if(source->position().isTemporary()){
        Position position(PositionType::TEMPORARY);

        auto var = std::make_shared<Variable>(name, source->type(), position); 
        storage[name] = var;
        return variables[name] = var;
    } else {
        return addVariable(name, source->type());
    }
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type, ast::Value& value){
    assert(type->is_const());

    Position position(PositionType::CONST);

    auto val = visit(ast::GetConstantValue(), value);
    
    auto var = std::make_shared<Variable>(variable, type, position, val);
    return variables[variable] = var;
}

std::shared_ptr<Variable> FunctionContext::addParameter(const std::string& parameter, std::shared_ptr<const Type> type){
    return variables[parameter] = newParameter(parameter, type);
}

std::shared_ptr<Variable> FunctionContext::newTemporary(){
    Position position(PositionType::TEMPORARY);

    std::string name = "ti_" + toString(temporary++);
    auto var = std::make_shared<Variable>(name, INT, position); 
    storage[name] = var;
    return variables[name] = var;
}

std::shared_ptr<Variable> FunctionContext::newFloatTemporary(){
    Position position(PositionType::TEMPORARY);

    std::string name = "tf_" + toString(temporary++);
    auto var = std::make_shared<Variable>(name, FLOAT, position); 
    storage[name] = var;
    return variables[name] = var;
}

void FunctionContext::storeTemporary(std::shared_ptr<Variable> temp){
    currentPosition -= temp->type()->size();

    Position position(PositionType::STACK, currentPosition + INT->size());
    
    temp->setPosition(position); 
}

void FunctionContext::reallocate_storage(){
    currentPosition = -INT->size();

    auto it = storage.begin();
    auto end = storage.end();

    while(it != end){
        auto v = it->second;

        if(v->position().isStack()){
            currentPosition -= v->type()->size();
            Position position(PositionType::STACK, currentPosition + INT->size());
            v->setPosition(position);
        }

        ++it;
    }
}

void FunctionContext::allocate_in_register(std::shared_ptr<Variable> variable, unsigned int register_){
    assert(variable->position().isStack()); 

    Position position(PositionType::REGISTER, register_);
    variable->setPosition(position);

    reallocate_storage();
}

void FunctionContext::allocate_in_param_register(std::shared_ptr<Variable> variable, unsigned int register_){
    assert(variable->position().isParameter()); 

    Position position(PositionType::PARAM_REGISTER, register_);
    variable->setPosition(position);

    reallocate_storage();
}

void FunctionContext::removeVariable(const std::string& variable){
    auto var = storage[variable];
    
    storage.erase(variable);

    //If its a temporary, no need to recalculate positions
    if(!var->position().isTemporary()){
        reallocate_storage();
    }
}
