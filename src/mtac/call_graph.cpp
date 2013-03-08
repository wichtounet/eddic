//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Function.hpp"
#include "GlobalContext.hpp"

#include "mtac/call_graph.hpp"
#include "mtac/Program.hpp"
#include "mtac/Function.hpp"

using namespace eddic;
        
mtac::call_graph::~call_graph(){
    for(auto& node_key : nodes){
        auto& node = node_key.second;

        for(auto& edge : node->in_edges){
            edge->source = nullptr;
            edge->target = nullptr;
        }
        
        for(auto& edge : node->out_edges){
            edge->source = nullptr;
            edge->target = nullptr;
        }

        node->in_edges.clear();
        node->out_edges.clear();
    }
}

mtac::call_graph_node_p mtac::call_graph::node(eddic::Function& function){
    auto it = nodes.find(function.mangled_name());

    if(it == nodes.end()){
        auto node = std::make_shared<mtac::call_graph_node>(function);
        nodes[function.mangled_name()] = node;
        return node;
    }

    return it->second;
}
        
mtac::call_graph_edge_p mtac::call_graph::edge(eddic::Function& source, eddic::Function& target){
    auto source_node = node(source);
    auto target_node = node(target);

    for(auto& edge : source_node->out_edges){
        if(edge->target == target_node){
            return edge;
        }
    }

    return nullptr;
}
        
void mtac::call_graph::add_edge(eddic::Function& source, eddic::Function& target){
    auto edge = this->edge(source, target);
    
    if(!edge){
        auto source_node = node(source);
        auto target_node = node(target);

        edge = std::make_shared<mtac::call_graph_edge>(source_node, target_node);

        source_node->out_edges.push_back(edge);
        target_node->in_edges.push_back(edge);
    }
    
    ++edge->count;
}

void compute_reachable(mtac::Reachable& reachable, mtac::call_graph_node_p node){
    if(reachable.find(node->function) == reachable.end()){
        reachable.insert(node->function);

        for(auto& edge : node->out_edges){
            if(edge->count > 0){
                compute_reachable(reachable, edge->target);
            }
        }
    }
}

void mtac::call_graph::compute_reachable(){
    eddic_assert(entry, "The call graph must be built before computing reachable");

    release_reachable();

    ::compute_reachable(reachable, entry);
}

void mtac::call_graph::release_reachable(){
    reachable.clear();
}

bool mtac::call_graph::is_reachable(eddic::Function& function){
    return reachable.find(function) != reachable.end();
}

void mtac::build_call_graph(mtac::Program& program){
    timing_timer timer(program.context->timing(), "build_cg");

    auto& cg = program.call_graph;

    for(auto& function : program){
        for(auto& block : function){
            for(auto& quadruple : block){
                if(quadruple.op == mtac::Operator::CALL){
                    cg.add_edge(function.definition(), quadruple.function());
                }
            }
        }

        if(function.is_main()){
            cg.entry = cg.node(function.definition());
        }
    }
}

void post_dfs_visit(mtac::call_graph_node_p& node, std::vector<std::reference_wrapper<eddic::Function>>& order){
    for(auto& edge : node->out_edges){
        if(edge->target->function != node->function){
            post_dfs_visit(edge->target, order);
        }
    }

    order.push_back(node->function);
}
        
std::vector<std::reference_wrapper<eddic::Function>> mtac::call_graph::topological_order(){
    std::vector<std::reference_wrapper<eddic::Function>> order;   

    post_dfs_visit(entry, order);

    return order;
}
