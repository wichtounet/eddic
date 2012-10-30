//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONTROL_FLOW_GRAPH_H
#define MTAC_CONTROL_FLOW_GRAPH_H

#include <memory>

namespace eddic {

namespace mtac {

class Function;
class BasicBlock;

void make_edge(std::shared_ptr<mtac::BasicBlock> from, std::shared_ptr<mtac::BasicBlock> to);
void remove_edge(std::shared_ptr<mtac::BasicBlock> from, std::shared_ptr<mtac::BasicBlock> to);

void build_control_flow_graph(std::shared_ptr<Function> function);

} //end of mtac

} //end of eddic

#endif
