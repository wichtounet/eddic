//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_AGGREGATES_H
#define LTAC_AGGREGATES_H

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

/*!
 * \brief Allocate aggregates variables into their own stack positions for the 
 * LTAC compiler. 
 * \param program The MTAC Program
 */
void allocate_aggregates(mtac::Program& program);

} //end of ltac

} //end of eddic

#endif
