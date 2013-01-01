//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
