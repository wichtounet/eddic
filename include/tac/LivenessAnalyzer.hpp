//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_LIVENESS_ANALYZER_H
#define TAC_LIVENESS_ANALYZER_H

#include "tac/Program.hpp"

namespace eddic {

namespace tac {

struct LivenessAnalyzer {
    void compute(tac::Program& program);
};

} //end of ast

} //end of eddic

#endif
