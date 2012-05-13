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
        //Init all but ENTRY
        if(graph[*it].block->index != -1){
            OUT[graph[*it].block] = problem.Init();
        }
    }

    bool changes = true;
    while(changes){
        for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
            auto vertex = *it;

            //Do not consider ENTRY
            if(graph[vertex].block->index == -1){
                continue;
            }

            ControlFlowGraph::OutEdgeIterator oit, oend;
            for(boost::tie(oit, oend) = boost::out_edges(vertex, graph); oit != oend; ++oit){
                auto edge = *oit;
                auto vertex_target = boost::target(edge, graph);
                auto successor = graph[vertex_target].block;

                IN[graph[vertex].block] = problem.meet(IN[graph[vertex].block], OUT[successor]);
                OUT[graph[vertex].block] = problem.transfer(graph[vertex].block, IN[graph[vertex].block]);
            }
        }
    }
}

template<bool Forward, typename Domain>
void backward_data_flow(std::shared_ptr<ControlFlowGraph>/* graph*/, DataFlowProblem<Forward, Domain>&/* problem*/){
    //TODO
}

} //end of mtac

} //end of eddic

#endif
