//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "logging.hpp"

#include "mtac/Loop.hpp"
#include "mtac/complete_loop_peeling.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Program.hpp"

using namespace eddic;

bool mtac::complete_loop_peeling::gate(std::shared_ptr<Configuration> configuration){
    return configuration->option_defined("fcomplete-peel-loops");
}

bool mtac::complete_loop_peeling::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;
    
    auto lit = iterate(function.loops());

    while(lit.has_next()){
        auto loop = *lit;

        if(loop.has_estimate() && loop.blocks().size() == 1){
            auto it = loop.estimate();

            if(it > 0 && it < 12){
                auto bb = *loop.begin();

                optimized = true;

                LOG<Trace>("Loops") << "Peel completely the loop with " << it << " iterations" << log::endl;
                function.context->global()->stats().inc_counter("loop_peeled");
                
                auto& statements = bb->statements;

                //The comparison is not necessary anymore
                statements.pop_back();

                int limit = statements.size();
                
                //There are perhaps new references to functions
                for(auto& statement : statements){
                    if(statement.op == mtac::Operator::CALL){
                        program.call_graph.edge(function.definition(), statement.function())->count += (it - 1);
                    }
                }

                //Save enough space for the new statements
                statements.reserve(limit * it);

                //Start at 1 because there is already the original body
                for(int i = 1; i < it; ++i){
                    for(int j = 0; j < limit; ++j){
                        statements.push_back(statements[j]); 
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
