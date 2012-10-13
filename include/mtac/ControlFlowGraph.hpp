//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONTROL_FLOW_GRAPH_H
#define MTAC_CONTROL_FLOW_GRAPH_H

#include <memory>

#include "boost_cfg.hpp"
#include <boost/graph/adjacency_list.hpp>

#include "mtac/BasicBlock.hpp"

namespace eddic {

namespace mtac {

class Function;

void build_control_flow_graph(std::shared_ptr<Function> function);

} //end of mtac

} //end of eddic

#endif
