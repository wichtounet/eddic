//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "Platform.hpp"
#include "SymbolTable.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"

#include "mtac/RegisterAllocation.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

void mtac::register_param_allocation(){
    PlatformDescriptor* descriptor = getPlatformDescriptor(platform);

    for(auto function_info : symbols){
        auto function = function_info.second;

        //Only custom functions have a context
        if(function->context){
            for(unsigned int i = 0; i < function->parameters.size(); ++i){
                auto& parameter = function->parameters[i];
                auto type = parameter.paramType;
                unsigned int position = function->getParameterPositionByType(parameter.name);
                auto param = function->context->getVariable(parameter.name);

                if((type == INT && position <= descriptor->numberOfIntParamRegisters()) || (type == FLOAT && position <= descriptor->numberOfFloatParamRegisters())){
                    Position oldPosition = param->position();

                    function->context->allocate_in_param_register(param, position);
                    
                    //We have to change the position of the all the following parameters
                    for(unsigned int j = i + 1; j < function->parameters.size(); ++j){
                        auto p = function->context->getVariable(function->parameters[j].name);
                        Position paramPosition = p->position();
                        p->setPosition(oldPosition); 
                        oldPosition = paramPosition;
                    }
                }
            }
        }
    }
}

typedef std::set<std::shared_ptr<Variable>> Candidates;

void search_candidates(mtac::VariableUsage& usage, Candidates& candidates, std::shared_ptr<Variable> variable, unsigned int variables){
    if(variables == 0){
        return;
    }

    if(candidates.size() < variables){
        candidates.insert(variable);
    } else {
        std::shared_ptr<Variable> min_var;

        for(auto& v : candidates){
            if(!min_var){
                min_var = v;
            } else if(usage[v] < usage[min_var]){
                min_var = v;
            }
        }

        if(usage[variable] > usage[min_var]){
            candidates.erase(min_var);
            candidates.insert(variable);
        }
    }
}

void mtac::register_variable_allocation(std::shared_ptr<mtac::Program> program){
    PlatformDescriptor* descriptor = getPlatformDescriptor(platform);

    if(descriptor->number_of_variable_registers() > 0 || descriptor->number_of_float_variable_registers() > 0){
        for(auto function : program->functions){
            auto usage = mtac::compute_variable_usage(function);

            Candidates int_var;
            Candidates float_var;

            for(auto variable_pair : function->context->stored_variables()){
                auto variable = variable_pair.second;

                if(variable->position().isStack()){
                    if(usage[variable] > 0){
                        if(variable->type() == INT){
                            search_candidates(usage, int_var, variable, descriptor->number_of_variable_registers());
                        } else if(variable->type() == FLOAT){
                            search_candidates(usage, float_var, variable, descriptor->number_of_float_variable_registers());
                        }
                    }
                }
            }

            unsigned int position = 0;
            for(auto variable : int_var){
                function->context->allocate_in_register(variable, ++position);
            }

            position = 0;
            for(auto variable : float_var){
                function->context->allocate_in_register(variable, ++position);
            }
        }
    }
}
