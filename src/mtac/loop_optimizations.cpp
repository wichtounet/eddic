//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "iterators.hpp"

#include "mtac/loop_optimizations.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

typedef mtac::ControlFlowGraph::InternalControlFlowGraph G;
typedef std::set<mtac::ControlFlowGraph::BasicBlockInfo> Loop;

struct Usage {
    std::unordered_map<std::shared_ptr<Variable>, unsigned int> written;
};

Usage compute_usage(const Loop& loop, const G& g){
    Usage usage;

    for(auto& vertex : loop){
        auto bb = g[vertex].block;
        
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(mtac::erase_result((*ptr)->op)){
                    ++(usage.written[(*ptr)->result]);
                }
            }
        }
    }

    return usage;
}

bool is_invariant(boost::optional<mtac::Argument>& argument, Usage& usage){
    if(argument){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*argument)){
            return usage.written[*ptr] == 0;
        }
    }

    return true;
}

bool is_invariant(mtac::Statement& statement, Usage& usage){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        if(mtac::erase_result(quadruple->op)){
            //If there are more than one write to this variable, the computation is not invariant
            if(usage.written[quadruple->result] > 1){
                return false;
            }

            return is_invariant(quadruple->arg1, usage) && is_invariant(quadruple->arg2, usage);
        }

        return false;
    }

    return false;
}

std::shared_ptr<mtac::BasicBlock> create_pre_header(const Loop& loop, std::shared_ptr<mtac::Function> function, const G& g){
    //Create a new basic block and detach it from the function
    auto pre_header = function->newBasicBlock();
    function->getBasicBlocks().pop_back();
    
    auto first_bb = g[*loop.begin()].block;

    auto bit = iterate(function->getBasicBlocks());

    while(bit.has_next()){
        if(*bit == first_bb){
            bit.insert(pre_header);

            break;
        }

        ++bit;
    }

    //TODO There are certainly cases when this is not enough
    
    return pre_header;
}

bool loop_invariant_code_motion(const Loop& loop, std::shared_ptr<mtac::Function> function, const G& g){
    std::shared_ptr<mtac::BasicBlock> pre_header;

    auto usage = compute_usage(loop, g);

    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        auto it = iterate(bb->statements); 

        while(it.has_next()){
            auto statement = *it;

            if(is_invariant(statement, usage)){
                //TODO Test if the invariant can be moved to the preheader
                
                //Create the preheader if necessary
                if(!pre_header){
                    pre_header = create_pre_header(loop, function, g);
                }
                    
                it.erase();
                pre_header->statements.push_back(statement);
            }

            ++it;
        }
    }

    return false;
}

} //end of anonymous namespace

bool mtac::loop_invariant_code_motion(std::shared_ptr<mtac::Function> function){
    auto graph = mtac::build_control_flow_graph(function);
    auto g = graph->get_graph();

    auto natural_loops = find_natural_loops(g);

    if(natural_loops.empty()){
        return false;
    }

    bool optimized = false;

    for(auto& loop : natural_loops){
        optimized |= ::loop_invariant_code_motion(loop, function, g);
    }
    
    return optimized;
}
