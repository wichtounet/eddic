//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Platform.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/RegisterAllocation.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

void mtac::register_param_allocation(std::shared_ptr<mtac::Program> program, Platform platform){
    auto descriptor = getPlatformDescriptor(platform);

    auto maxInt = descriptor->numberOfIntParamRegisters();
    auto maxFloat = descriptor->numberOfFloatParamRegisters();

    for(auto function_info : program->context->functions()){
        auto function = function_info.second;

        //Only custom functions have a context
        if(function->context){
            for(unsigned int i = 0; i < function->parameters.size(); ++i){
                auto& parameter = function->parameters[i];
                auto type = parameter.paramType;
                unsigned int position = function->getParameterPositionByType(parameter.name);
                auto param = function->context->getVariable(parameter.name);

                if((mtac::is_single_int_register(type) && position <= maxInt) || (mtac::is_single_float_register(type) && position <= maxFloat)){
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
