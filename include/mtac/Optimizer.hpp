//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_OPTIMIZER_H
#define MTAC_OPTIMIZER_H

#include <memory>

#include "Platform.hpp"
#include "Options.hpp"

#include "mtac/forward.hpp"

namespace eddic {

struct StringPool;

namespace mtac {

struct Optimizer {
    void optimize(mtac::Program& program, std::shared_ptr<StringPool> pool, Platform platform, std::shared_ptr<Configuration> configuration) const ;
};

} //end of mtac

} //end of eddic

#endif
