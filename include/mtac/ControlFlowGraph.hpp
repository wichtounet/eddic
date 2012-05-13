//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONTROL_FLOW_GRAPH_H
#define MTAC_CONTROL_FLOW_GRAPH_H

#include <memory>

#include <boost/graph/adjacency_list.hpp>

#include "mtac/BasicBlock.hpp"

namespace eddic {

namespace mtac {

struct vertex_info {
    std::shared_ptr<BasicBlock> block;
};

enum class EdgeType : unsigned int {
    COMMON
};

struct edge_info {
    EdgeType type;
};

class ControlFlowGraph {
    public:
        typedef boost::property<boost::graph_name_t, std::string> GraphProperties;

        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, vertex_info, edge_info, GraphProperties> InternalControlFlowGraph;
        
        //Iterators
        typedef boost::graph_traits<InternalControlFlowGraph>::edge_iterator EdgeIterator;
        typedef boost::graph_traits<InternalControlFlowGraph>::vertex_iterator BasicBlockIterator;
        typedef boost::graph_traits<InternalControlFlowGraph>::out_edge_iterator OutEdgeIterator;
        typedef boost::graph_traits<InternalControlFlowGraph>::adjacency_iterator AdjacentBasicBlockIterator;

        //Descriptors
        typedef boost::graph_traits<InternalControlFlowGraph>::edge_descriptor BasicBlockInfo;
        typedef boost::graph_traits<InternalControlFlowGraph>::vertex_descriptor EdgeInfo;
    
    public:
        ControlFlowGraph();

        std::pair<BasicBlockIterator, BasicBlockIterator> blocks();
        std::pair<EdgeIterator, EdgeIterator> edges();
        
    private:
        std::shared_ptr<InternalControlFlowGraph> graph;
};

} //end of mtac

} //end of eddic

#endif
