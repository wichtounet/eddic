//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "iterators.hpp"
#include "logging.hpp"
#include "GlobalContext.hpp"

#include "mtac/remove_unused_functions.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

typedef std::unordered_set<std::reference_wrapper<eddic::Function>> Reachable;

namespace std {
    std::hash<std::string> hasher;

    template<>
    class hash<std::reference_wrapper<eddic::Function>> {
    public:
        size_t operator()(const std::reference_wrapper<eddic::Function>& val) const {
            return hasher(val.get().mangled_name());
        }
    };
}

namespace {

void remove_references(mtac::Function& function){
    for(auto& bb : function){
        for(auto& quadruple : bb->statements){
            if(quadruple.op == mtac::Operator::CALL){
                --quadruple.function().references();
            }
        }
    }
}

void compute_reachable(Reachable reachable, mtac::call_graph_node_p node){
    if(reachable.find(node->function) == reachable.end()){
        reachable.insert(node->function);

        for(auto& edge : node->out_edges){
            compute_reachable(reachable, edge->target);
        }
    }
}

} //end of anonymous namespace

bool mtac::remove_unused_functions::operator()(mtac::Program& program){
    Reachable reachable;

    compute_reachable(reachable, program.call_graph.entry);

    auto it = iterate(program.functions);

    while(it.has_next()){
        auto& function = *it;

        if(reachable.find(function.definition()) == reachable.end()){
            //TODO remove_references(function);
            LOG<Debug>("Optimizer") << "Remove unused function " << function.get_name() << log::endl;
            it.erase();
            continue;
        } 

        ++it;
    }

    //Not necessary to restart the other passes
    return false;
}
