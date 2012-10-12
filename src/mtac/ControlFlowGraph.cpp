//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Function.hpp"

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

void make_edge(std::shared_ptr<mtac::BasicBlock> from, std::shared_ptr<mtac::BasicBlock> to){
    from->successors.push_back(to);
    to->predecessors.push_back(from);
}

void mtac::build_control_flow_graph(std::shared_ptr<mtac::Function> function){
    //Add the edges
    for(auto& block : function){
        //Get the following block
        auto next = block->next;
        
        //ENTRY
        if(block->index == -1){
            make_edge(block, next);
        }
        //EXIT
        else if(block->index == -2){
            //Nothing to do
        }
        //Empty block
        else if(block->statements.size() == 0){
            make_edge(block, next);
        }
        //Standard block
        else {
            auto& last_statement = block->statements.back();

            //If and IfFalse have two possible successors
            if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&last_statement)){
                make_edge(block, (*ptr)->block);
                make_edge(block, next);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&last_statement)){
                make_edge(block, (*ptr)->block);
                make_edge(block, next);
            } 
            //Goto has one possible successor
            else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last_statement)){
                make_edge(block, (*ptr)->block);
            }
            //All the other statements have only the fall through successor
            else {
                make_edge(block, next);
            }
        }
    }
}
