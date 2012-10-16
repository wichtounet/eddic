//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"

#include "ltac/aggregates.hpp"

using namespace eddic;

void ltac::allocate_aggregates(std::shared_ptr<mtac::Program> program){
    auto global_context = program->context;

    for(auto& function_pair : global_context->functions()){
        auto& function_context = function_pair.second->context;

        //Consider only user functions
        if(function_context){

        }
    }
}
