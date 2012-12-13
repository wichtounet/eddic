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

#include "mtac/EscapeAnalysis.hpp"
#include "mtac/Program.hpp"

#include "ltac/aggregates.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

void ltac::allocate_aggregates(mtac::program_p program){
    auto global_context = program->context;
    auto platform = global_context->target_platform();

    for(auto& function : program->functions){
        auto& function_context = function->context;

        //Consider only user functions
        if(function_context){
            int current_position = -INT->size(platform); 

            auto escaped = mtac::escape_analysis(function);

            for(auto& variable : function_context->stored_variables()){
                auto position = variable->position();
                auto type = variable->type();

                if(
                            !variable->is_reference() 
                        &&  (position.is_temporary() || position.is_variable()) 
                        &&  (type == STRING || type->is_template_type() || type->is_array() || type->is_custom_type() || escaped->count(variable))){
                    current_position -= type->size(platform);

                    Position position(PositionType::STACK, current_position + INT->size(platform));
                    variable->setPosition(position);
                }
            }

            function_context->set_stack_position(current_position);
        }
    }
}
