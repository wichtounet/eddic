//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/GlobalOptimizations.hpp"

#include "ltac/RegisterAllocator.hpp"
#include "ltac/LiveRegistersProblem.hpp"
#include "ltac/interference_graph.hpp"

using namespace eddic;

namespace {

void build_interference_graph(ltac::interference_graph& graph, std::shared_ptr<mtac::Function> function, std::shared_ptr<mtac::DataFlowResults<mtac::Domain<ltac::LiveRegisterValues>>> live_results){
    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            auto& live_registers = live_results->OUT_LS[statement].values().registers;

            if(live_registers.size() > 1){
                auto it = live_registers.begin();
                auto end = live_registers.end();

                while(it != end){
                    auto next = it;
                    ++next;

                    while(next != end){
                        graph.add_edge(it->reg, next->reg);

                        ++next;
                    }

                    ++it;
                }
            }
        }
    }

    graph.build_adjacency_vectors();
}

} //end of anonymous namespace

void ltac::register_allocation(std::shared_ptr<mtac::Program> program, Platform platform){
    for(auto& function : program->functions){
        //Compute Liveness
        ltac::LiveRegistersProblem problem;
        auto live_results = mtac::data_flow(function, problem);
    
        std::size_t size = function->pseudo_registers(); 
        interference_graph graph(size);
        build_interference_graph(graph, function, live_results);
    }
}
