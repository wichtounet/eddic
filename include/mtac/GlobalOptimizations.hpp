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

template<bool Forward, typename DomainValues>
void data_flow(std::shared_ptr<ControlFlowGraph> graph, DataFlowProblem<Forward, DomainValues>& problem){
    if(Forward){
        forward_data_flow(graph, problem);
    } else {
        backward_data_flow(graph, problem);
    }
}

template<bool Forward, typename DomainValues>
void forward_data_flow(std::shared_ptr<ControlFlowGraph> cfg, DataFlowProblem<Forward, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto graph = cfg->get_graph();

    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> OUT;
    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, Domain> IN;
    
    std::unordered_map<mtac::Statement, Domain> OUT_S;
    std::unordered_map<mtac::Statement, Domain> IN_S;
   
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
        changes = false;

        for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
            auto vertex = *it;
            auto B = graph[vertex].block;

            //Do not consider ENTRY
            if(B->index == -1){
                continue;
            }

            ControlFlowGraph::InEdgeIterator iit, iend;
            for(boost::tie(iit, iend) = boost::in_edges(vertex, graph); iit != iend; ++iit){
                auto edge = *iit;
                auto predecessor = boost::source(edge, graph);
                auto P = graph[predecessor].block;

                IN[B] = problem.meet(IN[B], OUT[P]);

                auto& statements = B->statements;

                IN_S[statements.front()] = IN[B];

                for(unsigned i = 0; i < statements.size(); ++i){
                    auto& statement = statements[i];

                    OUT_S[statement] = problem.transfer(statement, IN_S[statement]);

                    //The entry value of the next statement are the exit values of the current statement
                    if(i != statements.size() - 1){
                        IN_S[statements[i+1]] = OUT_S[statement];
                    }
                }

                OUT[B] = OUT_S[statements.back()];
            }

            //TODO Calculate changes
        }
    }
}

template<bool Forward, typename DomainValues>
void backward_data_flow(std::shared_ptr<ControlFlowGraph>/* graph*/, DataFlowProblem<Forward, DomainValues>&/* problem*/){
    typedef mtac::Domain<DomainValues> Domain;

    //TODO
}

} //end of mtac

} //end of eddic

#endif
