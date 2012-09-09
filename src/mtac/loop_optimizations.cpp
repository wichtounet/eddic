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
#include "mtac/Printer.hpp"

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

bool loop_invariant_code_motion(const Loop& loop, const G& g){
    std::shared_ptr<mtac::BasicBlock> pre_header;

    auto usage = compute_usage(loop, g);

    for(auto& vertex : loop){
        auto bb = g[vertex].block;

        auto it = iterate(bb->statements); 

        while(it.has_next()){
            auto statement = *it;

            if(is_invariant(statement, usage)){
                mtac::Printer printer;
                printer.printStatement(statement);
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
        optimized |= ::loop_invariant_code_motion(loop, g);
    }
    
    return optimized;
}
