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

#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Program.hpp"
#include "mtac/DataFlowProblem.hpp"

#define DEBUG_GLOBAL_ENABLED false
#define DEBUG_GLOBAL if(DEBUG_GLOBAL_ENABLED)

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
    
    DEBUG_GLOBAL std::cout << "OUT[" << *cfg->entry() << "] set to " << OUT[cfg->entry()] << std::endl;

    ControlFlowGraph::BasicBlockIterator it, end;
    for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
        //Init all but ENTRY
        if(graph[*it].block->index != -1){
            OUT[graph[*it].block] = problem.Init(function);
            DEBUG_GLOBAL std::cout << "OUT[" << *graph[*it].block << "] set to " << OUT[graph[*it].block] << std::endl;
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

                DEBUG_GLOBAL std::cout << "Meet B = " << *B << " with P = " << *P << std::endl;
                DEBUG_GLOBAL std::cout << "IN[B] before " << IN[B] << std::endl;
                DEBUG_GLOBAL std::cout << "OUT[P] before " << OUT[P] << std::endl;

                assign(IN[B], problem.meet(IN[B], OUT[P]), changes);
                
                DEBUG_GLOBAL std::cout << "IN[B] after " << IN[B] << std::endl;

                auto& statements = B->statements;

                if(statements.size() > 0){
                    assign(IN_S[statements.front()], IN[B], changes);

                    for(unsigned i = 0; i < statements.size(); ++i){
                        auto& statement = statements[i];

                        assign(OUT_S[statement], problem.transfer(B, statement, IN_S[statement]), changes);

                        //The entry value of the next statement are the exit values of the current statement
                        if(i != statements.size() - 1){
                            assign(IN_S[statements[i+1]], OUT_S[statement], changes);
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
    DEBUG_GLOBAL std::cout << "IN[" << *cfg->exit() << "] set to " << IN[cfg->exit()] << std::endl;

    ControlFlowGraph::BasicBlockIterator it, end;
    for(boost::tie(it,end) = boost::vertices(graph); it != end; ++it){
        auto block = graph[*it].block;

        //Init all but EXIT
        if(block->index != -2){
            IN[block] = problem.Init(function);
            DEBUG_GLOBAL std::cout << "IN[" << *block << "] set to " << IN[block] << std::endl;
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

                DEBUG_GLOBAL std::cout << "Meet B = " << *B << " with S = " << *S << std::endl;
                DEBUG_GLOBAL std::cout << "OUT[B] before " << OUT[B] << std::endl;
                DEBUG_GLOBAL std::cout << "IN[S] before " << IN[S] << std::endl;

                OUT[B] = problem.meet(OUT[B], IN[S]);
                
                DEBUG_GLOBAL std::cout << "OUT[B] after " << OUT[B] << std::endl;

                auto& statements = B->statements;

                if(statements.size() > 0){
                    assign(OUT_S[statements.back()], OUT[B], changes);

                    for(unsigned i = statements.size() - 1; i > 0; --i){
                        auto& statement = statements[i];

                        DEBUG_GLOBAL std::cout << "IN_S[" << i << "] before transfer " << IN_S[statement] << std::endl;
                        assign(IN_S[statement], problem.transfer(B, statement, OUT_S[statement]), changes);
                        DEBUG_GLOBAL std::cout << "IN_S[" << i << "] after transfer " << IN_S[statement] << std::endl;
                            
                        OUT_S[statements[i-1]] = IN_S[statement];
                    }
                        
                    assign(IN_S[statements[0]], problem.transfer(B, statements[0], OUT_S[statements[0]]), changes);
                    
                    assign(IN[B], IN_S[statements.front()], changes);
                } else {
                    //If the basic block is empty, the IN values are the OUT values
                    assign(IN[B], OUT[B], changes);
                }
                
                DEBUG_GLOBAL std::cout << "IN[B] after transfer " << IN[B] << std::endl;
            }
        }
    }

    return results;
}

} //end of mtac

} //end of eddic

#endif
