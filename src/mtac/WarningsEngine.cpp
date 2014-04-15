//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Warnings.hpp"
#include "Options.hpp"

#include "mtac/WarningsEngine.hpp"
#include "mtac/Program.hpp"

using namespace eddic;

void mtac::collect_warnings(mtac::Program& program, std::shared_ptr<Configuration> configuration){
    if(configuration->option_defined("warning-unused")){
        for(auto& function : program.functions){
            if(program.call_graph.node(function.definition())->in_edges.size() == 0 && !function.is_main()){
                warn("Unused function: " + function.get_name());
            }
        }
    }
}
