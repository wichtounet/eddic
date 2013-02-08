//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "assert.hpp"

#include "mtac/loop.hpp"
#include "mtac/basic_block.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Function.hpp"

using namespace eddic;

mtac::loop::loop(const std::set<mtac::basic_block_p>& blocks) : m_blocks(blocks) {
    //Nothing
}

mtac::loop::iterator mtac::loop::begin(){
    return m_blocks.begin();
}

mtac::loop::iterator mtac::loop::end(){
    return m_blocks.end();
}

bool mtac::loop::has_estimate(){
    return m_estimate > 0;
}

long& mtac::loop::estimate(){
    return m_estimate;
}

long& mtac::loop::initial_value(){
    return m_initial;
}
      
std::set<mtac::basic_block_p>& mtac::loop::blocks(){
    return m_blocks;
}
      
const std::set<mtac::basic_block_p>& mtac::loop::blocks() const {
    return m_blocks;
}

mtac::InductionVariables& mtac::loop::basic_induction_variables(){
    return biv;
}

mtac::InductionVariables& mtac::loop::dependent_induction_variables(){
    return div;
}

mtac::basic_block_p mtac::find_entry(mtac::loop& loop){
    for(auto& block : loop.blocks()){
        for(auto& pred : block->predecessors){
            if(loop.blocks().find(pred) == loop.blocks().end()){
                LOG<Trace>("Control-Flow") << "Found " << *block << " as entry of " << loop << log::endl;

                return block;
            }
        }
    }

    eddic_unreachable("Every loop should have a single entry");
}
        
bool mtac::loop::single_exit() const {
    int exits = 0;
    
    for(auto& block : blocks()){
        for(auto& succ : block->successors){
            if(blocks().find(succ) == blocks().end()){
                ++exits;
            }
        }
    }

    return exits == 1;
}

mtac::basic_block_p mtac::find_exit(mtac::loop& loop){
    for(auto& block : loop.blocks()){
        for(auto& succ : block->successors){
            if(loop.blocks().find(succ) == loop.blocks().end()){
                LOG<Trace>("Control-Flow") << "Found " << *block << " as exit of " << loop << log::endl;

                return block;
            }
        }
    }

    eddic_unreachable("Every loop should have at least an exit");
}

mtac::basic_block_p mtac::find_preheader(mtac::loop& loop){
    auto first_bb = find_entry(loop);

    for(auto& pred : first_bb->predecessors){
        if(loop.blocks().find(pred) == loop.blocks().end()){
            LOG<Trace>("Control-Flow") << "Found " << *pred << " as preheader of " << loop << log::endl;

            return pred;
        }
    }

    return nullptr;
}

mtac::basic_block_p mtac::find_safe_preheader(mtac::loop& loop, mtac::Function& function, bool create){
    auto first_bb = find_entry(loop);

    //Step 1: Try to find if there is already a preheader
    
    auto pred = find_preheader(loop);

    if(pred){
        //It must be the only successor and a fall through edge
        if(pred->successors.size() == 1 && pred->next == first_bb){
            LOG<Trace>("Control-Flow") << "Found " << *pred << " as safe preheader of " << loop << log::endl;

            return pred;
        }
    }

    if(!create){
        return nullptr;
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
                
    LOG<Trace>("Control-Flow") << "Create " << *pre_header << " as safe preheader of " << loop << log::endl;
    
    return pre_header;
}

std::ostream& mtac::operator<<(std::ostream& stream, const mtac::loop& loop){
    stream << "loop {";

    for(auto& block : loop.blocks()){
        stream << block << ", ";
    }

    return stream << "}";
}
