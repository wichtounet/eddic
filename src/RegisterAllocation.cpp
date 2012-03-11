//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Compiler.hpp"
#include "Platform.hpp"
#include "RegisterAllocation.hpp"
#include "FunctionTable.hpp"
#include "FunctionContext.hpp"

using namespace eddic;

void eddic::allocateParams(FunctionTable& functionTable){
    PlatformDescriptor* descriptor = getPlatformDescriptor(platform);

    auto it = functionTable.begin();
    auto end = functionTable.end();

    while(it != end){
        auto function = it->second;

        if(function->context){
            for(auto parameter : function->parameters){
                auto type = parameter.paramType;
                unsigned int position = function->getParameterPositionByType(parameter.name);
                auto param = function->context->getVariable(parameter.name);

                if(type == BaseType::INT && position <= descriptor->numberOfIntParamRegisters()){
                    //TODO Set the variable position to register
                } else if(type == BaseType::FLOAT && position <= descriptor->numberOfFloatParamRegisters()){
                    //TODO Set the variable position to register
                }
            }
        }

        ++it;
    }
}
