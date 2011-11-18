//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <functional>
#include <utility>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>

#include "GlobalContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"

#include "il/IntermediateProgram.hpp"

#include "GetConstantValue.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;

using namespace eddic;
        
void GlobalContext::writeIL(IntermediateProgram& program){
    for(auto it : variables){
        Type type = it.second->type();

        if(type.isArray()){
            program.addInstruction(program.factory().createGlobalArray(it.second->position().name(), type.base(), type.size()));
        } else {
            if (type.base() == BaseType::INT) {
                program.addInstruction(program.factory().createGlobalIntVariable(it.second->position().name(), boost::get<int>(it.second->val())));
            } else if (type.base() == BaseType::STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
                program.addInstruction(program.factory().createGlobalStringVariable(it.second->position().name(), value.first, value.second));
            }
        }
    }
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, Type type){
    //A global variable must have a value
    assert(type.isArray());
    
    Position position(GLOBAL, variable);
    
    return variables[variable] = std::make_shared<Variable>(variable, type, position);
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, Type type, ast::Value& value){
    Position position(GLOBAL, variable);

    auto val = boost::apply_visitor(GetConstantValue(), value);

    return variables[variable] = std::make_shared<Variable>(variable, type, position, val);
}
