//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Function.hpp"
#include "mtac/basic_block.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;
        
void mtac::make_edge(mtac::basic_block_p from, mtac::basic_block_p to){
    from->successors.push_back(to);
    to->predecessors.push_back(from);
}

void mtac::remove_edge(mtac::basic_block_p from, mtac::basic_block_p to){
    from->successors.erase(std::remove(from->successors.begin(), from->successors.end(), to), from->successors.end());
    to->predecessors.erase(std::remove(to->predecessors.begin(), to->predecessors.end(), from), to->predecessors.end());
}

void mtac::build_control_flow_graph(mtac::Function& function){
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
            auto& quadruple = block->statements.back();

            if(quadruple.op == mtac::Operator::GOTO){
                make_edge(block, quadruple.block);
            } else if(quadruple.is_if() || quadruple.is_if_false()){ 
                make_edge(block, quadruple.block);
                make_edge(block, next);
            } else {
                make_edge(block, next);
            }
        }
    }
}
