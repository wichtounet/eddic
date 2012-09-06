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
typedef mtac::ControlFlowGraph::EdgeInfo Edge;
typedef mtac::ControlFlowGraph::BasicBlockInfo Vertex;
typedef boost::property_map<mtac::ControlFlowGraph::InternalControlFlowGraph, boost::vertex_index_t>::type IndexMap;
typedef boost::iterator_property_map<std::vector<Vertex>::iterator, IndexMap> PredMap;

void mtac::loop_analysis(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        auto graph = mtac::build_control_flow_graph(function);
        auto g = graph->get_graph();
        
        std::vector<Vertex> domTreePredVector = std::vector<Vertex>(boost::num_vertices(g), boost::graph_traits<G>::null_vertex());
        PredMap domTreePredMap = boost::make_iterator_property_map(domTreePredVector.begin(), boost::get(boost::vertex_index, g));

        boost::lengauer_tarjan_dominator_tree(g, boost::vertex(0, g), domTreePredMap);

        std::vector<Edge> back_edges;
        
        ControlFlowGraph::EdgeIterator it, end;
        for(boost::tie(it,end) = boost::edges(g); it != end; ++it){
            auto source = boost::source(*it, g);
            auto target = boost::target(*it, g);

            //A node dominates itself
            if(source == target){
                back_edges.push_back(*it);
            } else {
                if(boost::get(domTreePredMap, source) != boost::graph_traits<G>::null_vertex()){
                    auto dominator = boost::get(domTreePredMap,source);

                    if(dominator == target){
                        back_edges.push_back(*it);
                    }
                }
            }
        }

        std::vector<std::set<Vertex>> natural_loops;

        //Get all edges n -> d
        for(auto& back_edge : back_edges){
            std::set<Vertex> natural_loop;
            natural_loop.insert(boost::target(back_edge, g));

            //Add all predecessors of d in the set
            //
            
            natural_loops.push_back(natural_loop);
        }

        std::cout << "Found " << natural_loops.size() << " natural loops" << std::endl;
    }
}
