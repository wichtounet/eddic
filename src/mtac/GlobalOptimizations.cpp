//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/GlobalOptimizations.hpp"

using namespace eddic;

std::shared_ptr<mtac::ControlFlowGraph> build_control_flow_graph(std::shared_ptr<mtac::Function> function){
    auto g = std::make_shared<mtac::ControlFlowGraph>();
    auto& graph = g->get_graph();

    std::unordered_map<std::shared_ptr<mtac::BasicBlock>, mtac::ControlFlowGraph::BasicBlockInfo> infos;
    
    //Create all the vertices
    mtac::Function::BlockIterator it, end;
    for(boost::tie(it, end) = function->blocks(); it != end; ++it){
        auto& block = *it;

        auto block_vertex = boost::add_vertex(graph);
        graph[block_vertex].block = block;
        infos[block] = block_vertex;
    }
    
    //Set the entry and exit blocks
    g->entry() = function->getBasicBlocks().front();
    g->exit() = function->getBasicBlocks().back();
    
    //Add the edges
    for(boost::tie(it, end) = function->blocks(); it != end; ++it){
        auto& block = *it;
        
        //Get the following block
        auto next = it;
        ++next;
        
        //ENTRY
        if(block->index == -1){
            boost::add_edge(infos[block], infos[*next], graph);
        }
        //EXIT
        else if(block->index == -2){
            //Nothing to do
        }
        //Standard block
        else {
            auto& last_statement = block->statements.back();

            //If and IfFalse have two possible successors
            if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&last_statement)){
                boost::add_edge(infos[block], infos[(*ptr)->block], graph);
                boost::add_edge(infos[block], infos[*next], graph);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&last_statement)){
                boost::add_edge(infos[block], infos[(*ptr)->block], graph);
                boost::add_edge(infos[block], infos[*next], graph);
            } 
            //Goto has one possible sucessor
            else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last_statement)){
                boost::add_edge(infos[block], infos[(*ptr)->block], graph);
            }
            //All the other statements have only the fallback successor
            else {
                boost::add_edge(infos[block], infos[*next], graph);
            }
        }
    }
    
    return g;
}
