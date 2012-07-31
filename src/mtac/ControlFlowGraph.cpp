//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

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
        
std::shared_ptr<mtac::BasicBlock> mtac::ControlFlowGraph::entry(){
    return entry_block;
}

std::shared_ptr<mtac::BasicBlock> mtac::ControlFlowGraph::exit(){
    return exit_block;
}
