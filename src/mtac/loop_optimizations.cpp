//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/loop_optimizations.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/ControlFlowGraph.hpp"

using namespace eddic;

bool mtac::loop_invariant_code_motion(std::shared_ptr<mtac::Function> function){
    auto graph = mtac::build_control_flow_graph(function);
    auto g = graph->get_graph();

    auto natural_loops = find_natural_loops(g);

    if(natural_loops.empty()){
        return false;
    }
    
    return false;
}
