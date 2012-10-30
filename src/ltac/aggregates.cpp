//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Platform.hpp"
#include "Type.hpp"
#include "Variable.hpp"

#include "ltac/aggregates.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

void ltac::allocate_aggregates(std::shared_ptr<mtac::Program> program){
    auto global_context = program->context;
    auto platform = global_context->target_platform();

    for(auto& function_pair : global_context->functions()){
        auto& function_context = function_pair.second->context;

        //Consider only user functions
        if(function_context){
            int current_position = -INT->size(platform); 

            for(auto variable : function_context->stored_variables()){
                auto position = variable->position();
                auto type = variable->type();

                if((position.is_temporary() || position.is_variable()) && (type == STRING || type->is_template() || type->is_array() || type->is_custom_type())){
                    current_position -= type->size(platform);

                    Position position(PositionType::STACK, current_position + INT->size(platform));
                    variable->setPosition(position);
                }
            }

            function_context->set_stack_position(current_position);
        }
    }
}
