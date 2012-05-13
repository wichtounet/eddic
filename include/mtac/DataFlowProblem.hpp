//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DataOPTIMIZER_H
#define MTAC_OPTIMIZER_H

#include "mtac/Program.hpp"

namespace eddic {

class StringPool;

namespace mtac {

template<bool Forward, typename Domain>
struct DataFlowProblem {
    //What about statement ?
    //What about reading the basic blocks ?

    Domain meet(Domain& values);
    Domain transfer(Domain& values);

    void Boundary(Domain& values);
    void Init(Domain& values);
};

} //end of mtac

} //end of eddic

#endif
