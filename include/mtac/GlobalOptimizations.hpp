//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_GLOBAL_OPTIMIZATIONS_H
#define MTAC_GLOBAL_OPTIMIZATIONS_H

#include <memory>

#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Program.hpp"
#include "mtac/DataFlowProblem.hpp"

namespace eddic {

namespace mtac {

std::shared_ptr<ControlFlowGraph> build_control_flow_graph(std::shared_ptr<Function> function);

template<bool Forward, typename Domain>
void data_flow(std::shared_ptr<ControlFlowGraph> graph, DataFlowProblem<Forward, Domain>& problem){
    if(Forward){
        forward_data_flow(graph, problem);
    } else {
        backward_data_flow(graph, problem);
    }
}

template<bool Forward, typename Domain>
void forward_data_flow(std::shared_ptr<ControlFlowGraph> cfg, DataFlowProblem<Forward, Domain>& problem){
    auto graph = cfg->get_graph();

    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> OUT;
    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> IN;
   
    OUT[cfg->entry()] = problem.Boundary();

    ControlFlowGraph::BasicBlockIterator it, end;
    for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
        OUT[graph[*it].block] = problem.Init();
    }

    //TODO
}

template<bool Forward, typename Domain>
void backward_data_flow(std::shared_ptr<ControlFlowGraph>/* graph*/, DataFlowProblem<Forward, Domain>&/* problem*/){
    //TODO
}

} //end of mtac

} //end of eddic

#endif
