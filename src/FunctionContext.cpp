//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/apply_visitor.hpp>

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "GetConstantValue.hpp"

using namespace eddic;

int FunctionContext::size() const {
    return currentPosition - 4;
}

std::shared_ptr<Variable> FunctionContext::newParameter(const std::string& variable, Type type){
    Position position(PositionType::PARAMETER, currentParameter);

    currentParameter += ::size(type);

    return std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> FunctionContext::newVariable(const std::string& variable, Type type){
    Position position(PositionType::STACK, currentPosition);

    currentPosition += ::size(type);

    return std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, Type type){
    return variables[variable] = newVariable(variable, type);
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, Type type, ast::Value& value){
    assert(type.isConst());

    Position position(PositionType::CONST);

    auto val = boost::apply_visitor(GetConstantValue(), value);

    return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
}

std::shared_ptr<Variable> FunctionContext::addParameter(const std::string& parameter, Type type){
    return variables[parameter] = newParameter(parameter, type);
}

std::shared_ptr<Variable> FunctionContext::newTemporary(){
    Position position(TEMPORARY);

    std::string name = "t_" + toString(temporary++);
    Type type(BaseType::INT, false); 

    return variables[name] = std::make_shared<Variable>(name, type, position); 
}
