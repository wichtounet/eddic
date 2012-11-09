//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
    void optimize(mtac::program_p program, std::shared_ptr<StringPool> pool, Platform platform, std::shared_ptr<Configuration> configuration) const ;
};

} //end of mtac

} //end of eddic

#endif
