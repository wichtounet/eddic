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
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> data_flow(std::shared_ptr<ControlFlowGraph> graph, DataFlowProblem<Forward, DomainValues>& problem){
    if(Forward){
        return forward_data_flow(graph, problem);
    } else {
        return backward_data_flow(graph, problem);
    }
}

//TODO Find a way to improve it in order to pass value by reference
template<typename Values>
inline void assign(Values& old, Values value, bool& changes){
    if(old.top()){
        changes = !value.top();
    } else if(old.values().size() != value.values().size()){
        changes = true;
    }

    old = value;
}

template<bool Forward, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> forward_data_flow(std::shared_ptr<ControlFlowGraph> cfg, DataFlowProblem<Forward, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto graph = cfg->get_graph();

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;
   
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

                assign(IN[B], problem.meet(IN[B], OUT[P]), changes);

                auto& statements = B->statements;

                assign(IN_S[statements.front()], IN[B], changes);

                for(unsigned i = 0; i < statements.size(); ++i){
                    auto& statement = statements[i];

                    assign(OUT_S[statement], problem.transfer(statement, IN_S[statement]), changes);

                    //The entry value of the next statement are the exit values of the current statement
                    if(i != statements.size() - 1){
                        assign(IN_S[statements[i+1]], OUT_S[statement], changes);
                    }
                }

                assign(OUT[B], OUT_S[statements.back()], changes);
            }
        }
    }

    return results;
}

template<bool Forward, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>>  backward_data_flow(std::shared_ptr<ControlFlowGraph>/* graph*/, DataFlowProblem<Forward, DomainValues>&/* problem*/){
    typedef mtac::Domain<DomainValues> Domain;

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    //TODO

    return results;
}

} //end of mtac

} //end of eddic

#endif
