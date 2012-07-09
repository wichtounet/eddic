//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/DeadCodeElimination.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::dead_code_elimination(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    mtac::LiveVariableAnalysisProblem problem;
    auto results = mtac::data_flow(function, problem);

    for(auto& block : function->getBasicBlocks()){
        auto it = block->statements.begin();
        auto end = block->statements.end();

        while(it != end){
            auto statement = *it;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(mtac::erase_result((*ptr)->op)){
                    if(results->OUT_S[statement].values().find((*ptr)->result) == results->OUT_S[statement].values().end()){
                        it = block->statements.erase(it);
                        end = block->statements.end();
                        optimized=true;
                        continue;
                    }
                }
            }

            ++it;
        }
    }

    return optimized;
}
