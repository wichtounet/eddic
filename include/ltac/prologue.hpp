//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_PROLOGUE_H
#define LTAC_PROLOGUE_H

#include <memory>

#include "Options.hpp"

#include "mtac/forward.hpp"

namespace eddic {

namespace ltac {

void generate_prologue_epilogue(mtac::Program& program, std::shared_ptr<Configuration> configuration);

} //end of ltac

} //end of eddic

#endif
