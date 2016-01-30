//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/Utils.hpp"
#include "mtac/EscapeAnalysis.hpp"
#include "mtac/clean_variables.hpp"
#include "mtac/variable_usage.hpp"

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
