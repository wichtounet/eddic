//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OPTIMIZER_H
#define MTAC_OPTIMIZER_H

#include "Platform.hpp"

#include "mtac/Program.hpp"

namespace eddic {

class StringPool;

namespace mtac {

struct Optimizer {
    void optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> pool, Platform platform) const ;
    void basic_optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> pool) const ;
};

} //end of mtac

} //end of eddic

#endif
