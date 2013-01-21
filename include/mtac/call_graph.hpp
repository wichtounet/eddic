//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CALL_GRAPH_H
#define MTAC_CALL_GRAPH_H

#include<vector>
#include<memory>

namespace eddic {

namespace mtac {

struct Program;

struct call_graph_node;
typedef std::shared_ptr<call_graph_node> call_graph_node_p;

struct call_graph_edge {
    call_graph_node_p source = nullptr;
    call_graph_node_p target = nullptr;
    std::size_t count = 0;
};

typedef std::shared_ptr<call_graph_edge> call_graph_edge_p;

struct call_graph_node {
    eddic::Function& function;
    std::vector<call_graph_edge_p> out_edges;
    std::vector<call_graph_edge_p> in_edges;
};

struct call_graph {
    call_graph_node_p entry = nullptr;
};

void build_call_graph(mtac::Program& program);

} //end of mtac

} //end of eddic

#endif

