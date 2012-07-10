//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Platform.hpp"
#include "RegisterAllocation.hpp"
#include "SymbolTable.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"

using namespace eddic;

void eddic::allocateParams(){
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

                Position oldPosition = param->position();

                if((type == INT && position <= descriptor->numberOfIntParamRegisters()) || (type == FLOAT && position <= descriptor->numberOfFloatParamRegisters())){
                    Position paramPosition(PositionType::PARAM_REGISTER, position);
                    param->setPosition(paramPosition);
                }

                //If the parameter has been changed
                if(param->position().isParamRegister()){
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
