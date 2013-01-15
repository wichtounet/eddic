//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "logging.hpp"

#include "mtac/Loop.hpp"
#include "mtac/remove_empty_loops.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::remove_empty_loops::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;
    
    auto lit = iterate(function.loops());

    while(lit.has_next()){
        auto loop = *lit;

        if(loop.has_estimate()){
            auto it = loop.estimate();
            auto bb = *loop.begin();

            if(bb->statements.size() == 2){
                auto first = bb->statements.front();

                auto& basic_induction_variables = loop.basic_induction_variables();
                if(basic_induction_variables.find(first.result) != basic_induction_variables.end()){
                    auto linear_equation = basic_induction_variables.begin()->second;
                    auto initial_value = loop.initial_value();

                    bool loop_removed = false;

                    //The loop does not iterate
                    if(it == 0){
                        bb->statements.clear();
                        loop_removed = true;
                    } else if(it > 0){
                        bb->statements.clear();
                        loop_removed = true;

                        bb->emplace_back(first.result, static_cast<int>(initial_value + it * linear_equation.d), mtac::Operator::ASSIGN);
                    }

                    if(loop_removed){
                        function.context->global()->stats().inc_counter("empty_loop_removed");

                        //It is not a loop anymore
                        mtac::remove_edge(bb, bb);

                        lit.erase();

                        optimized = true;

                        continue;
                    }
                }
            }
        }

        ++lit;
    }

    return optimized;
}
