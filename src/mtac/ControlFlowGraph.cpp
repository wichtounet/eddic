//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Function.hpp"
#include "mtac/BasicBlock.hpp"
#include "mtac/Statement.hpp"

using namespace eddic;
        
void mtac::make_edge(mtac::basic_block_p from, mtac::basic_block_p to){
    from->successors.push_back(to);
    to->predecessors.push_back(from);
}

void mtac::remove_edge(mtac::basic_block_p from, mtac::basic_block_p to){
    auto sit = iterate(from->successors);

    while(sit.has_next()){
        if(*sit == to){
            sit.erase();
            break;
        }

        ++sit;
    }
    
    auto pit = iterate(to->predecessors);

    while(pit.has_next()){
        if(*pit == from){
            pit.erase();
            break;
        }

        ++pit;
    }
}

void mtac::build_control_flow_graph(std::shared_ptr<mtac::Function> function){
    //Destroy the CFG
    for(auto& block : function){
        block->successors.clear();
        block->predecessors.clear();
    }

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
