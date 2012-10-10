//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "mtac/ControlFlowGraph.hpp"

using namespace eddic;
        
mtac::ControlFlowGraph::ControlFlowGraph(){
    //Nothing to init
}

std::pair<mtac::ControlFlowGraph::BasicBlockIterator, mtac::ControlFlowGraph::BasicBlockIterator> mtac::ControlFlowGraph::blocks(){
    return boost::vertices(graph);
}

std::pair<mtac::ControlFlowGraph::EdgeIterator, mtac::ControlFlowGraph::EdgeIterator> mtac::ControlFlowGraph::edges(){
    return boost::edges(graph);
}

mtac::ControlFlowGraph::InternalControlFlowGraph& mtac::ControlFlowGraph::get_graph(){
    return graph;
}
        
std::shared_ptr<mtac::BasicBlock>& mtac::ControlFlowGraph::entry(){
    return entry_block;
}

std::shared_ptr<mtac::BasicBlock>& mtac::ControlFlowGraph::exit(){
    return exit_block;
}

std::shared_ptr<mtac::ControlFlowGraph> mtac::build_control_flow_graph(std::shared_ptr<mtac::Function> function){
    auto g = std::make_shared<mtac::ControlFlowGraph>();
    auto& graph = g->get_graph();

    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, mtac::ControlFlowGraph::BasicBlockInfo> infos;
    
    //Create all the vertices
    for(auto& block : function){
        auto block_vertex = boost::add_vertex(graph);
        graph[block_vertex].block = block;
        infos[block] = block_vertex;
    }
    
    //Set the entry and exit blocks
    g->entry() = function->entry_bb();
    g->exit() = function->exit_bb();
    
    //Add the edges
    for(auto& block : function){
        //Get the following block
        auto next = block->next;
        
        //ENTRY
        if(block->index == -1){
            boost::add_edge(infos[block], infos[next], graph);
        }
        //EXIT
        else if(block->index == -2){
            //Nothing to do
        }
        //Empty block
        else if(block->statements.size() == 0){
            boost::add_edge(infos[block], infos[next], graph);
        }
        //Standard block
        else {
            auto& last_statement = block->statements.back();

            //If and IfFalse have two possible successors
            if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&last_statement)){
                boost::add_edge(infos[block], infos[(*ptr)->block], graph);
                boost::add_edge(infos[block], infos[next], graph);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&last_statement)){
                boost::add_edge(infos[block], infos[(*ptr)->block], graph);
                boost::add_edge(infos[block], infos[next], graph);
            } 
            //Goto has one possible sucessor
            else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last_statement)){
                boost::add_edge(infos[block], infos[(*ptr)->block], graph);
            }
            //All the other statements have only the fallback successor
            else {
                boost::add_edge(infos[block], infos[next], graph);
            }
        }
    }
    
    return g;
}
