//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_WARNINGS_ENGINE_H
#define MTAC_WARNINGS_ENGINE_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

struct Configuration;

namespace mtac {

void collect_warnings(mtac::Program& program, std::shared_ptr<Configuration> configuration);

} //end of mtac

} //end of eddic

#endif
