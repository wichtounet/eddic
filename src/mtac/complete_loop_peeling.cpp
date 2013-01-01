//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"

#include "mtac/Loop.hpp"
#include "mtac/complete_loop_peeling.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::complete_loop_peeling::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;
    
    auto lit = iterate(function.loops());

    while(lit.has_next()){
        auto loop = *lit;

        if(loop->has_estimate()){
            auto it = loop->estimate();

            if(it > 0 && it < 12){
                auto bb = *loop->begin();

                optimized = true;

                function.context->global()->stats().inc_counter("loop_peeled");

                //The comparison is not necessary anymore
                bb->statements.pop_back();

                auto statements = bb->statements;

                //Start at 1 because there is already the original body
                for(int i = 1; i < it; ++i){
                    for(auto& statement : statements){
                        bb->statements.push_back(mtac::copy(statement, function.context->global())); 
                    }
                }

                //It is not a loop anymore
                mtac::remove_edge(bb, bb);

                lit.erase();

                continue;
            }
        }

        ++lit;
    }

    return optimized;
}
