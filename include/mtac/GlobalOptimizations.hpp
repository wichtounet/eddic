//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_GLOBAL_OPTIMIZATIONS_H
#define MTAC_GLOBAL_OPTIMIZATIONS_H

#include <memory>
#include <iostream>

#include "assert.hpp"
#include "logging.hpp"

#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Program.hpp"
#include "mtac/DataFlowProblem.hpp"

#include "PerfsTimer.hpp"

namespace eddic {

namespace mtac {

std::shared_ptr<ControlFlowGraph> build_control_flow_graph(std::shared_ptr<Function> function);

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> data_flow(std::shared_ptr<mtac::Function> function, DataFlowProblem<Type, DomainValues>& problem){
    if(Type == DataFlowType::Forward){
        auto graph = mtac::build_control_flow_graph(function);
        return forward_data_flow(function, graph, problem);
    } else if(Type == DataFlowType::Backward){
        auto graph = mtac::build_control_flow_graph(function);
        return backward_data_flow(function, graph, problem);
    } else {
        ASSERT_PATH_NOT_TAKEN("This data-flow type is not handled");
    }
}

template<typename Left, typename Right>
inline void assign(Left& old, Right&& value, bool& changes){
    if(old.top()){
        changes = !value.top();
    } else if(old.values().size() != value.values().size()){
        changes = true;
    }

    old = value;
}

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> forward_data_flow(std::shared_ptr<mtac::Function> function, std::shared_ptr<ControlFlowGraph> cfg, DataFlowProblem<Type, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto graph = cfg->get_graph();

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    problem.Gather(function);
   
    OUT[cfg->entry()] = problem.Boundary(function);
    
    log::emit<Debug>("Data-Flow") << "OUT[" << *cfg->entry() << "] set to " << OUT[cfg->entry()] << log::endl;

    ControlFlowGraph::BasicBlockIterator it, end;
    for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
        //Init all but ENTRY
        if(graph[*it].block->index != -1){
            OUT[graph[*it].block] = problem.Init(function);
            log::emit<Debug>("Data-Flow") << "OUT[" << *graph[*it].block << "] set to " << OUT[graph[*it].block] << log::endl;
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

                log::emit<Debug>("Data-Flow") << "Meet B = " << *B << " with P = " << *P << log::endl;
                log::emit<Debug>("Data-Flow") << "IN[B] before " << IN[B] << log::endl;
                log::emit<Debug>("Data-Flow") << "OUT[P] before " << OUT[P] << log::endl;

                IN[B] = problem.meet(IN[B], OUT[P]);
                
                log::emit<Debug>("Data-Flow") << "IN[B] after " << IN[B] << log::endl;

                auto& statements = B->statements;

                if(statements.size() > 0){
                    IN_S[statements.front()] = IN[B];

                    for(unsigned i = 0; i < statements.size(); ++i){
                        auto& statement = statements[i];

                        assign(OUT_S[statement], problem.transfer(B, statement, IN_S[statement]), changes);

                        //The entry value of the next statement are the exit values of the current statement
                        if(i != statements.size() - 1){
                            IN_S[statements[i+1]] = OUT_S[statement];
                        }
                    }
                    
                    assign(OUT[B], OUT_S[statements.back()], changes);
                } else {
                    //If the basic block is empty, the OUT values are the IN values
                    assign(OUT[B], IN[B], changes);
                }
            }
        }
    }

    return results;
}

template<DataFlowType Type, typename DomainValues>
std::shared_ptr<DataFlowResults<mtac::Domain<DomainValues>>> backward_data_flow(std::shared_ptr<mtac::Function> function, std::shared_ptr<ControlFlowGraph> cfg, DataFlowProblem<Type, DomainValues>& problem){
    typedef mtac::Domain<DomainValues> Domain;

    auto graph = cfg->get_graph();

    auto results = std::make_shared<DataFlowResults<Domain>>();
    
    auto& OUT = results->OUT;
    auto& IN = results->IN;
    
    auto& OUT_S = results->OUT_S;
    auto& IN_S = results->IN_S;

    problem.Gather(function);
   
    IN[cfg->exit()] = problem.Boundary(function);
    log::emit<Debug>("Data-Flow") << "IN[" << *cfg->exit() << "] set to " << IN[cfg->exit()] << log::endl;

    ControlFlowGraph::BasicBlockIterator it, end;
    for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
        auto block = graph[*it].block;

        //Init all but EXIT
        if(block->index != -2){
            IN[block] = problem.Init(function);
            log::emit<Debug>("Data-Flow") << "IN[" << *block << "] set to " << IN[block] << log::endl;
        }
    }

    bool changes = true;
    while(changes){
        changes = false;

        for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
            auto vertex = *it;
            auto B = graph[vertex].block;

            //Do not consider EXIT
            if(B->index == -2){
                continue;
            }

            ControlFlowGraph::OutEdgeIterator oit, oend;
            for(boost::tie(oit, oend) = boost::out_edges(vertex, graph); oit != oend; ++oit){
                auto edge = *oit;
                auto successor = boost::target(edge, graph);
                auto S = graph[successor].block;

                /*if(S->index == B->index){
                    continue;
                }*/

                log::emit<Debug>("Data-Flow") << "Meet B = " << *B << " with S = " << *S << log::endl;
                log::emit<Debug>("Data-Flow") << "OUT[B] before " << OUT[B] << log::endl;
                log::emit<Debug>("Data-Flow") << "IN[S] before " << IN[S] << log::endl;

                OUT[B] = problem.meet(OUT[B], IN[S]);
                
                log::emit<Debug>("Data-Flow") << "OUT[B] after " << OUT[B] << log::endl;

                auto& statements = B->statements;

                if(statements.size() > 0){
                    assign(OUT_S[statements.back()], OUT[B], changes);

                    for(unsigned i = statements.size() - 1; i > 0; --i){
                        auto& statement = statements[i];

                        log::emit<Debug>("Data-Flow") << "IN_S[" << i << "] before transfer " << IN_S[statement] << log::endl;
                        assign(IN_S[statement], problem.transfer(B, statement, OUT_S[statement]), changes);
                        log::emit<Debug>("Data-Flow") << "IN_S[" << i << "] after transfer " << IN_S[statement] << log::endl;
                            
                        OUT_S[statements[i-1]] = IN_S[statement];
                    }
                        
                    assign(IN_S[statements[0]], problem.transfer(B, statements[0], OUT_S[statements[0]]), changes);
                    
                    assign(IN[B], IN_S[statements.front()], changes);
                } else {
                    //If the basic block is empty, the IN values are the OUT values
                    assign(IN[B], OUT[B], changes);
                }
                
                log::emit<Debug>("Data-Flow") << "IN[B] after transfer " << IN[B] << log::endl;
            }
        }
    }

    return results;
}

} //end of mtac

} //end of eddic

#endif
