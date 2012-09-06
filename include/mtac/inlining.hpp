//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_INLINING_H
#define MTAC_INLINING_H

#include <memory>

#include "Options.hpp"

#include "mtac/Program.hpp"

namespace eddic {

namespace mtac {

bool inline_functions(std::shared_ptr<mtac::Program> program, std::shared_ptr<Configuration> configuration);

} //end of mtac

} //end of eddic

#endif
