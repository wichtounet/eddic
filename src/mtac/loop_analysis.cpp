//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <stack>

#include <boost/graph/dominator_tree.hpp>

#include "VisitorUtils.hpp"
#include "logging.hpp"

#include "mtac/LoopAnalysis.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/ControlFlowGraph.hpp"

using namespace eddic;

namespace {

typedef mtac::ControlFlowGraph::InternalControlFlowGraph G;
typedef mtac::ControlFlowGraph::EdgeInfo Edge;
typedef mtac::ControlFlowGraph::BasicBlockInfo Vertex;
typedef boost::property_map<mtac::ControlFlowGraph::InternalControlFlowGraph, boost::vertex_index_t>::type IndexMap;
typedef boost::iterator_property_map<std::vector<Vertex>::iterator, IndexMap> PredMap;

struct DepthInit : public boost::static_visitor<void> {
    void operator()(std::string){
        //Nothing
    }
    
    void operator()(std::shared_ptr<mtac::NoOp>){
        //Nothing
    }
    
    template<typename T>
    void operator()(T t){
        t->depth = 0;
    }
};

struct DepthIncrementer : public boost::static_visitor<void> {
    void operator()(std::string){
        //Nothing
    }
    
    void operator()(std::shared_ptr<mtac::NoOp>){
        //Nothing
    }
    
    template<typename T>
    void operator()(T t){
        ++t->depth;
    }
};

void init_depth(std::shared_ptr<mtac::BasicBlock> bb){
    DepthInit init;

    visit_each(init, bb->statements);
}

void increase_depth(std::shared_ptr<mtac::BasicBlock> bb){
    DepthIncrementer incrementer;

    visit_each(incrementer, bb->statements);
}

} //end of anonymous namespace

void mtac::loop_analysis(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        for(auto& bb : function->getBasicBlocks()){
            init_depth(bb);
        }

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

            auto n = boost::source(back_edge, g);
            auto d = boost::target(back_edge, g);

            natural_loop.insert(d);
            natural_loop.insert(n);

            log::emit<Trace>("Control-Flow") << "Back edge n = B" << g[n].block->index << log::endl;
            log::emit<Trace>("Control-Flow") << "Back edge d = B" << g[d].block->index << log::endl;

            if(n != d){
                std::stack<Vertex> vertices;
                vertices.push(n);

                while(!vertices.empty()){
                    auto vertex = vertices.top();
                    vertices.pop();

                    ControlFlowGraph::InEdgeIterator iit, iend;
                    for(boost::tie(iit, iend) = boost::in_edges(vertex, g); iit != iend; ++iit){
                        auto edge = *iit;

                        auto target = boost::source(edge, g);
                        auto source = boost::target(edge, g);

                        if(target != source && target != d && !natural_loop.count(target)){
                            natural_loop.insert(target);
                            vertices.push(target);
                        }
                    }
                }
            }

            log::emit<Trace>("Control-Flow") << "Natural loop of size " << natural_loop.size() << log::endl;
            
            natural_loops.push_back(natural_loop);
        }

        log::emit<Trace>("Control-Flow") << "Found " << natural_loops.size() << " natural loops" << log::endl;

        for(auto& loop : natural_loops){
            for(auto& parts : loop){
                auto bb = g[parts].block;

                increase_depth(bb);
            }
        }
    }
}
