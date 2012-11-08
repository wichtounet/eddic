//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <stack>

#include "VisitorUtils.hpp"
#include "logging.hpp"
#include "Variable.hpp"

#include "mtac/loop_analysis.hpp"
#include "mtac/dominators.hpp"
#include "mtac/Loop.hpp"
#include "mtac/Program.hpp"
#include "mtac/Statement.hpp"

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

void init_depth(mtac::basic_block_p bb){
    DepthInit init;

    bb->depth = 0;
    visit_each(init, bb->statements);
}

void increase_depth(mtac::basic_block_p bb){
    DepthIncrementer incrementer;

    ++bb->depth;
    visit_each(incrementer, bb->statements);
}

} //end of anonymous namespace

void mtac::full_loop_analysis(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        full_loop_analysis(function);
    }
}

void mtac::full_loop_analysis(mtac::function_p function){
    compute_dominators(function);

    for(auto& bb : function){
        init_depth(bb);
    }

    //Run the analysis on the function
    mtac::loop_analysis()(function);

    for(auto& loop : function->loops()){
        for(auto& bb : loop){
            increase_depth(bb);
        }
    }
}

bool mtac::loop_analysis::operator()(mtac::function_p function){
    std::vector<std::pair<mtac::basic_block_p, mtac::basic_block_p>> back_edges;

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

    function->loops().clear();

    //Get all edges n -> d
    for(auto& back_edge : back_edges){
        std::set<mtac::basic_block_p> natural_loop;

        auto n = back_edge.first;
        auto d = back_edge.first;

        natural_loop.insert(d);
        natural_loop.insert(n);

        log::emit<Trace>("Control-Flow") << "Back edge n = B" << n->index << log::endl;
        log::emit<Trace>("Control-Flow") << "Back edge d = B" << d->index << log::endl;

        if(n != d){
            std::stack<mtac::basic_block_p> vertices;
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

        auto loop = std::make_shared<mtac::Loop>(natural_loop);
        function->loops().push_back(loop);
    }

    log::emit<Trace>("Control-Flow") << "Found " << function->loops().size() << " natural loops" << log::endl;

    //Analysis only
    return false;
}
