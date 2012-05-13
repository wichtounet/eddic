//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/GlobalOptimizations.hpp"

using namespace eddic;

void mtac::forward_data_flow(std::shared_ptr<ControlFlowGraph> graph){
    //TODO
}

std::shared_ptr<mtac::ControlFlowGraph> build_control_flow_graph(std::shared_ptr<Function> function){
    auto graph = std::make_shared<mtac::ControlFlowGraph>();

    //TODO
    
    return graph;
}
