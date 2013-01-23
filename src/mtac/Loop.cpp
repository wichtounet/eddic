//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "assert.hpp"

#include "mtac/Loop.hpp"
#include "mtac/basic_block.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Function.hpp"

using namespace eddic;

mtac::Loop::Loop(const std::set<mtac::basic_block_p>& blocks) : m_blocks(blocks) {
    //Nothing
}

mtac::Loop::iterator mtac::Loop::begin(){
    return m_blocks.begin();
}

mtac::Loop::iterator mtac::Loop::end(){
    return m_blocks.end();
}

bool mtac::Loop::has_estimate(){
    return m_estimate > 0;
}

long& mtac::Loop::estimate(){
    return m_estimate;
}

long& mtac::Loop::initial_value(){
    return m_initial;
}
      
std::set<mtac::basic_block_p>& mtac::Loop::blocks(){
    return m_blocks;
}

mtac::InductionVariables& mtac::Loop::basic_induction_variables(){
    return biv;
}

mtac::InductionVariables& mtac::Loop::dependent_induction_variables(){
    return div;
}

mtac::basic_block_p mtac::find_entry(mtac::Loop& loop){
    for(auto& block : loop.blocks()){
        for(auto& pred : block->predecessors){
            if(loop.blocks().find(pred) == loop.blocks().end()){
                LOG<Trace>("Control-Flow") << "Found " << *block << " as entry of loop" << log::endl;

                return block;
            }
        }
    }

    eddic_unreachable("Every loop should have a single entry");
}

mtac::basic_block_p mtac::create_pre_header(mtac::Loop& loop, mtac::Function& function){
    auto first_bb = find_entry(loop);

    //Step 1: Try to find if there is already a preheader

    if(first_bb->predecessors.size() == 1){
        auto& pred = first_bb->predecessors.front();

        //It must be the only successor and a fall through edge
        if(pred->successors.size() == 1 && pred->next == first_bb){
            return pred;
        }
    }

    //Step 2: If not found, create a new preheader

    auto pre_header = function.new_bb();

    //Redispatch all the predecessors

    auto predecessors = first_bb->predecessors;
    for(auto& pred : predecessors){
        if(loop.blocks().find(pred) == loop.blocks().end()){
            mtac::remove_edge(pred, first_bb);
            mtac::make_edge(pred, pre_header);

            auto& quadruple = pred->statements.back();

            if(quadruple.block == first_bb){
                quadruple.block = pre_header;
            }
        }
    }

    function.insert_before(function.at(first_bb), pre_header);

    //Create the fall through edge
    mtac::make_edge(pre_header, first_bb);
                
    LOG<Trace>("Control-Flow") << "Create " << *pre_header << " as preheader of loop" << log::endl;
    
    return pre_header;
}

