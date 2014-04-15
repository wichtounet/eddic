//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_CONTROL_FLOW_GRAPH_H
#define MTAC_CONTROL_FLOW_GRAPH_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

void make_edge(mtac::basic_block_p from, mtac::basic_block_p to);
void remove_edge(mtac::basic_block_p from, mtac::basic_block_p to);

void build_control_flow_graph(mtac::Function& function);

} //end of mtac

} //end of eddic

#endif
