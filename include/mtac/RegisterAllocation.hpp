//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef REGISTER_ALLOCATION_H
#define REGISTER_ALLOCATION_H

#include "Platform.hpp"

namespace eddic {

namespace mtac {

class Program;
    
void register_param_allocation(std::shared_ptr<mtac::Program> program, Platform platform);

} //end of mtac

} //end of eddic

#endif
