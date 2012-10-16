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
#include "logging.hpp"

#include "ast/GetConstantValue.hpp"

using namespace eddic;

FunctionContext::FunctionContext(std::shared_ptr<Context> parent, std::shared_ptr<GlobalContext> global_context, Platform platform, std::shared_ptr<Configuration> configuration) : 
        Context(parent, global_context), platform(platform) {
    currentPosition = -INT->size(platform); 
    
    if(configuration->option_defined("fomit-frame-pointer")){
        currentParameter = INT->size(platform);
    } else {
        currentParameter = 2 * INT->size(platform);
    }
}

int FunctionContext::size() const {
    auto size = -currentPosition;

    if(size == -INT->size(platform)){
        return 0;
    }

    return size;
}

std::shared_ptr<Variable> FunctionContext::newParameter(const std::string& variable, std::shared_ptr<const Type> type){
    Position position(PositionType::PARAMETER, currentParameter);

    currentParameter += type->size(platform);

    return std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> FunctionContext::newVariable(const std::string& variable, std::shared_ptr<const Type> type){
    currentPosition -= type->size(platform);

    Position position(PositionType::STACK, currentPosition + INT->size(platform));
    auto var = std::make_shared<Variable>(variable, type, position);

    log::emit<Info>("Variables") << "Allocate " << variable << " at " << position.offset() << log::endl;

    storage.push_back(var);

    return var;
}

Storage FunctionContext::stored_variables(){
    return storage;
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type){
    return variables[variable] = newVariable(variable, type);
}

std::shared_ptr<Variable> FunctionContext::newVariable(std::shared_ptr<Variable> source){
    std::string name = "g_" + source->name() + "_" + toString(temporary++);
    
    if(source->position().is_temporary()){
        Position position(PositionType::TEMPORARY);

        auto var = std::make_shared<Variable>(name, source->type(), position); 
        storage.push_back(var);
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

std::shared_ptr<Variable> FunctionContext::generate_variable(const std::string& prefix, std::shared_ptr<const Type> type){
    std::string name = prefix + "_" + toString(generated++); 

    return addVariable(name, type);
}

std::shared_ptr<Variable> FunctionContext::addParameter(const std::string& parameter, std::shared_ptr<const Type> type){
    return variables[parameter] = newParameter(parameter, type);
}

std::shared_ptr<Variable> FunctionContext::new_temporary(std::shared_ptr<const Type> type){
    ASSERT((type->is_standard_type() && type != STRING) || type->is_pointer(), "Invalid temprary");

    Position position(PositionType::TEMPORARY);

    std::string name = "t_" + toString(temporary++);
    auto var = std::make_shared<Variable>(name, type, position); 
    storage.push_back(var);
    return variables[name] = var;
}

void FunctionContext::storeTemporary(std::shared_ptr<Variable> temp){
    currentPosition -= temp->type()->size(platform);

    Position position(PositionType::STACK, currentPosition + INT->size(platform));
    
    log::emit<Info>("Variables") << "Store temporary " << temp->name() << " at " << position.offset() << log::endl;
    
    temp->setPosition(position); 
}

void FunctionContext::reallocate_storage(){
    currentPosition = -INT->size(platform);

    for(auto& v : storage){
        if(v->position().isStack()){
            currentPosition -= v->type()->size(platform);
            Position position(PositionType::STACK, currentPosition + INT->size(platform));
            v->setPosition(position);
    
            log::emit<Info>("Variables") << "Reallocate " << v->name() << " at " << position.offset() << log::endl;
        }
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

void FunctionContext::removeVariable(std::shared_ptr<Variable> variable){
    auto iter_var = std::find(storage.begin(), storage.end(), variable);
    auto var = *iter_var;
    
    storage.erase(iter_var);
    
    log::emit<Info>("Variables") << "Remove " << variable->name() << log::endl;

    //If its a temporary, no need to recalculate positions
    if(!var->position().is_temporary()){
        reallocate_storage();
    }
}

std::shared_ptr<FunctionContext> FunctionContext::function(){
    return shared_from_this();
}
