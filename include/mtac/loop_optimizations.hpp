//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LOOP_OPTIMIZATIONS_H
#define MTAC_LOOP_OPTIMIZATIONS_H

#include <memory>

namespace eddic {

namespace mtac {

class Function;

bool loop_invariant_code_motion(std::shared_ptr<mtac::Function> function);
bool loop_induction_variables_optimization(std::shared_ptr<mtac::Function> function);
bool remove_empty_loops(std::shared_ptr<mtac::Function> function);

} //end of mtac

} //end of eddic

#endif
