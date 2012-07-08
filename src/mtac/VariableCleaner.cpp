//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_set>

#include "FunctionContext.hpp"
#include "Variable.hpp"

#include "mtac/VariableCleaner.hpp"

using namespace eddic;

template<typename T>
void collect(std::unordered_set<std::shared_ptr<Variable>>& usage, T arg){
    if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
        usage.insert(*variablePtr);
    }
}

template<typename T>
void collect_optional(std::unordered_set<std::shared_ptr<Variable>>& usage, T opt){
    if(opt){
        collect(usage, *opt);
    }
}

void eddic::mtac::clean_variables(std::shared_ptr<mtac::Function> function){
    std::unordered_set<std::shared_ptr<Variable>> usage;

    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                usage.insert((*ptr)->result);
                collect_optional(usage, (*ptr)->arg1);
                collect_optional(usage, (*ptr)->arg2);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
                collect(usage, (*ptr)->arg);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                collect(usage, (*ptr)->arg1);
                collect_optional(usage, (*ptr)->arg2);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                collect(usage, (*ptr)->arg1);
                collect_optional(usage, (*ptr)->arg2);
            }
        }
    }
    
    std::vector<std::shared_ptr<Variable>> unused;
    
    auto it = function->context->begin();
    auto end = function->context->end();

    while(it != end){
        auto variable = it->second;

        //Temporary and parameters are not interesting, because they dot not take any space
        if(!variable->position().isParameter() && !variable->position().isParamRegister()){
            if(usage.find(variable) == usage.end()){
                unused.push_back(variable);
            }
        }

        ++it;
    }

    for(auto& variable : unused){
        function->context->removeVariable(variable->name());
    }
}
