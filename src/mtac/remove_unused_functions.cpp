//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "iterators.hpp"
#include "logging.hpp"
#include "GlobalContext.hpp"

#include "mtac/remove_unused_functions.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::remove_unused_functions::operator()(mtac::Program& program){
    program.call_graph.compute_reachable();

    program.functions.erase(std::remove_if(program.functions.begin(), program.functions.end(), [&program](auto& function){
        if(!program.call_graph.is_reachable(function.definition())){
            LOG<Debug>("Optimizer") << "Remove unused function " << function.get_name() << log::endl;
            return true;
        } 

        return false;
    }), program.functions.end());

    program.call_graph.release_reachable();

    //Not necessary to restart the other passes
    return false;
}
