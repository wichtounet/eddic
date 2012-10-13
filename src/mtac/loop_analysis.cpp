//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <stack>

#include "VisitorUtils.hpp"
#include "logging.hpp"

#include "mtac/loop_analysis.hpp"
#include "mtac/dominators.hpp"

using namespace eddic;

namespace {

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

void mtac::full_loop_analysis(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        full_loop_analysis(function);
    }
}

void mtac::full_loop_analysis(std::shared_ptr<mtac::Function> function){
    compute_dominators(function);

    for(auto& bb : function){
        init_depth(bb);
    }

    auto natural_loops = find_natural_loops(function);

    for(auto& loop : natural_loops){
        for(auto& bb : loop){
            increase_depth(bb);
        }
    }
}

std::vector<std::set<std::shared_ptr<mtac::BasicBlock>>> mtac::find_natural_loops(std::shared_ptr<mtac::Function> function){
    std::vector<std::pair<std::shared_ptr<mtac::BasicBlock>, std::shared_ptr<mtac::BasicBlock>>> back_edges;

    for(auto& block : function){
        for(auto& succ : block->successors){
            //A node dominates itself
            if(block == succ){
                back_edges.push_back(std::make_pair(block,succ));
            } else {
                if(block->dominator == succ){
                    back_edges.push_back(std::make_pair(block,succ));
                }
            }
        }
    }

    std::vector<std::set<std::shared_ptr<mtac::BasicBlock>>> natural_loops;

    //Get all edges n -> d
    for(auto& back_edge : back_edges){
        std::set<std::shared_ptr<mtac::BasicBlock>> natural_loop;

        auto n = back_edge.first;
        auto d = back_edge.first;

        natural_loop.insert(d);
        natural_loop.insert(n);

        log::emit<Trace>("Control-Flow") << "Back edge n = B" << n->index << log::endl;
        log::emit<Trace>("Control-Flow") << "Back edge d = B" << d->index << log::endl;

        if(n != d){
            std::stack<std::shared_ptr<mtac::BasicBlock>> vertices;
            vertices.push(n);

            while(!vertices.empty()){
                auto source = vertices.top();
                vertices.pop();

                for(auto& target : source->predecessors){
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

    return natural_loops;
}
