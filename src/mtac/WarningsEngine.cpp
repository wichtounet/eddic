//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Warnings.hpp"
#include "Options.hpp"

#include "mtac/WarningsEngine.hpp"
#include "mtac/Program.hpp"

using namespace eddic;

void mtac::collect_warnings(mtac::Program& program, std::shared_ptr<Configuration> configuration){
    if(configuration->option_defined("warning-unused")){
        for(auto& function : program.functions){
            if(program.call_graph.node(function.definition())->in_edges.size() == 0){
                warn("Unused function: " + function.get_name());
            }
        }
    }
}
