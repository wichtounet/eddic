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
#include "mtac/Utils.hpp"

using namespace eddic;

void eddic::mtac::clean_variables(std::shared_ptr<mtac::Function> function){
    auto variable_usage = mtac::compute_variable_usage(function);
    
    std::vector<std::shared_ptr<Variable>> unused;
    for(auto variable_pair : function->context->stored_variables()){
        auto variable = variable_pair.second;

        //Temporary and parameters are not interesting, because they dot not take any space
        if(!variable->position().isParameter() && !variable->position().isParamRegister()){
            if(variable_usage[variable] > 0){
                unused.push_back(variable);
            }
        }
    }

    for(auto& variable : unused){
        function->context->removeVariable(variable->name());
    }
}
