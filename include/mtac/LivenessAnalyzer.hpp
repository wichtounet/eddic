//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LIVENESS_ANALYZER_H
#define MTAC_LIVENESS_ANALYZER_H

#include "mtac/Program.hpp"

namespace eddic {

namespace mtac {

struct LivenessAnalyzer {
    void compute(std::shared_ptr<mtac::Program> program);
};

} //end of ast

} //end of eddic

#endif
