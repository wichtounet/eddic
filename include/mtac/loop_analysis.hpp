//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LOOP_ANALYSIS_H
#define MTAC_LOOP_ANALYSIS_H

#include <memory>
#include <vector>
#include <set>

#include "mtac/Program.hpp"
#include "mtac/ControlFlowGraph.hpp"

namespace eddic {

namespace mtac {

void full_loop_analysis(std::shared_ptr<mtac::Program> program);
void full_loop_analysis(std::shared_ptr<mtac::Function> function);

std::vector<std::set<mtac::ControlFlowGraph::BasicBlockInfo>> find_natural_loops(mtac::ControlFlowGraph::InternalControlFlowGraph& g);

} //end of mtac

} //end of eddic

#endif
