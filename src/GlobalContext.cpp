//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "ast/GetConstantValue.hpp"

using namespace eddic;

std::unordered_map<std::string, std::shared_ptr<Variable>> GlobalContext::getVariables(){
    return variables;
}
        
GlobalContext::GlobalContext() : Context(NULL) {
    Val zero = 0;
    
    variables["_mem_start"] = std::make_shared<Variable>("_mem_start", new_type("int"), Position(PositionType::GLOBAL, "_mem_start"), zero);
    variables["_mem_start"]->addReference(); //In order to not display a warning

    variables["_mem_last"] = std::make_shared<Variable>("_mem_last", new_type("int"), Position(PositionType::GLOBAL, "_mem_last"), zero);
    variables["_mem_last"]->addReference(); //In order to not display a warning
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type){
    //Only global array have no value, other types have all values
    assert(type->is_array());

    Position position(PositionType::GLOBAL, variable);
    
    return variables[variable] = std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, std::shared_ptr<const Type> type, ast::Value& value){
    auto val = visit(ast::GetConstantValue(), value);
     
    if(type->is_const()){
        Position position(PositionType::CONST);
        return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
    }

    Position position(PositionType::GLOBAL, variable);
    return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
}
