//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/dominator_tree.hpp>

#include "mtac/LoopAnalysis.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/ControlFlowGraph.hpp"

using namespace eddic;

typedef mtac::ControlFlowGraph::InternalControlFlowGraph G;
typedef mtac::ControlFlowGraph::BasicBlockInfo Vertex;
typedef boost::property_map<mtac::ControlFlowGraph::InternalControlFlowGraph, boost::vertex_index_t>::type IndexMap;
typedef boost::iterator_property_map<std::vector<Vertex>::iterator, IndexMap> PredMap;

struct dfs_visitor : public boost::default_dfs_visitor {
    G& graph;

    dfs_visitor(G& graph) : graph(graph) {}

    template<typename U>
    void discover_vertex(U u, const G& g){
        auto new_vertex = add_vertex(graph);
        graph[new_vertex].block = g[u].block;
    }
    
    template<typename E>
    void tree_edge(E e, const G& g){
        add_edge(boost::source(e, g), boost::target(e, g), graph); 
    }
};

void mtac::loop_analysis(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        auto graph = mtac::build_control_flow_graph(function);
        auto g = graph->get_graph();
        
        std::vector<Vertex> domTreePredVector = std::vector<Vertex>(boost::num_vertices(g), boost::graph_traits<G>::null_vertex());
        PredMap domTreePredMap = boost::make_iterator_property_map(domTreePredVector.begin(), boost::get(boost::vertex_index, g));

        boost::lengauer_tarjan_dominator_tree(g, boost::vertex(0, g), domTreePredMap);
        
        ControlFlowGraph::EdgeIterator it, end;
        for(boost::tie(it,end) = boost::edges(g); it != end; ++it){
            auto source = boost::source(*it, g);
            auto target = boost::target(*it, g);

            //targets dominates source
            
            if(boost::get(domTreePredMap, source) != boost::graph_traits<G>::null_vertex()){
                auto dominator = boost::get(domTreePredMap,source);

                if(dominator == target){
                    std::cout << "Found back edge" << std::endl;
                }
            }
        }

        /*G depth_first_tree;
        dfs_visitor visitor(depth_first_tree);

        boost::depth_first_search(g, boost::visitor(visitor));

        std::cout << "Dominators found" << std::endl;*/
    }
}
