//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/GlobalOptimizations.hpp"

#include "ltac/RegisterAllocator.hpp"
#include "ltac/LiveRegistersProblem.hpp"
#include "ltac/interference_graph.hpp"

using namespace eddic;

void ltac::register_allocation(std::shared_ptr<mtac::Program> program, Platform platform){
    for(auto& function : program->functions){
        //Compute Liveness
        ltac::LiveRegistersProblem problem;
        auto live_results = mtac::data_flow(function, problem);

        //TODO Build interference graph
    
        std::size_t size = 100; //TODO Get that from the function
        interference_graph graph(size);
    }
}
