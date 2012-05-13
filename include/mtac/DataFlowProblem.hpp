//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DATA_FLOW_PROBLEM_H
#define MTAC_DATA_FLOW_PROBLEM_H

namespace eddic {

namespace mtac {

template<bool Forward, typename Domain>
struct DataFlowProblem {
    //What about statement ?
    //What about reading the basic blocks ?

    virtual Domain meet(Domain& values) = 0;
    virtual Domain transfer(Domain& values) = 0;

    virtual Domain Boundary() = 0;
    virtual Domain Init() = 0;
};

} //end of mtac

} //end of eddic

#endif
