//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_DOMINATORS_H
#define MTAC_DOMINATORS_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

/*!
 * \brief Compute the direct dominator of each basic_block.
 *
 * The computation is made with the Lengauer and Tarjan algorithm. The algorithm runs
 * in O(ma(m, n)) time where a(m, n) is the inverse of the Ackermann's function. 
 *
 * Reference: A Fast Algorithm for finding dominators in a Flowgraph by Thomas Lengauer
 * and Roberrt Endre Tarjan
 */
void compute_dominators(mtac::Function& function);

} //end of mtac

} //end of eddic

#endif
