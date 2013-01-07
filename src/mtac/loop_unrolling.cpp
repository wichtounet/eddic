//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"

#include "mtac/Function.hpp"
#include "mtac/Loop.hpp"
#include "mtac/loop_unrolling.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::loop_unrolling::gate(std::shared_ptr<Configuration> configuration){
    return configuration->option_defined("funroll-loops");
}

bool mtac::loop_unrolling::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;

    for(auto& loop : function.loops()){
        if(loop->has_estimate()){
            auto it = loop->estimate();

            if(it > 100){
                auto bb = *loop->begin();

                //Do not increase too much the size of the body
                if(bb->statements.size() < 20){
                    unsigned int factor = 0;
                    if((it % 8) == 0){
                        factor = 8;
                    } else if((it % 4) == 0){
                        factor = 4;
                    } else if((it % 2) == 0){
                        factor = 2;
                    }

                    if(factor == 0){
                        continue;
                    }

                    LOG<Trace>("Loops") << "Unroll the loop with a factor " << factor << log::endl;
                    function.context->global()->stats().inc_counter("loop_unrolled");

                    optimized = true;

                    //The comparison is not necessary here anymore
                    auto comparison = bb->statements.back();
                    bb->statements.pop_back();

                    auto statements = bb->statements;

                    //Start at 1 because there is already the original body
                    for(unsigned int i = 1; i < factor; ++i){
                        for(auto& statement : statements){
                            bb->statements.push_back(mtac::copy(statement)); 
                        }
                    }

                    //Put the comparison again at the end
                    bb->statements.push_back(comparison);
                }
            }
        }
    }

    return optimized;
}
