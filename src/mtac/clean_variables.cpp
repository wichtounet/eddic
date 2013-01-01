//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/Utils.hpp"
#include "mtac/EscapeAnalysis.hpp"
#include "mtac/Printer.hpp"
#include "mtac/Statement.hpp"
#include "mtac/clean_variables.hpp"

using namespace eddic;

bool mtac::clean_variables::operator()(mtac::Function& function){
    auto variable_usage = mtac::compute_variable_usage(function);
    
    std::vector<std::shared_ptr<Variable>> unused;
    for(auto& variable : function.context->stored_variables()){
        //Temporary and parameters are not interesting, because they dot not take any space
        if(!variable->position().isParameter() && !variable->position().isParamRegister()){
            if(variable_usage[variable] == 0){
                unused.push_back(variable);
            }
        }
    }

    for(auto& variable : unused){
        function.context->removeVariable(variable);
    }

    return false;
}
