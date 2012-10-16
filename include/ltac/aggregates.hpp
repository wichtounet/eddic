//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_AGGREGATES_H
#define LTAC_AGGREGATES_H

#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

/*!
 * \brief Allocate aggregates variables into their own stack positions for the 
 * LTAC compiler. 
 * \param program The MTAC Program
 */
void allocate_aggregates(std::shared_ptr<mtac::Program> program);

} //end of ltac

} //end of eddic

#endif
