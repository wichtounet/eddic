//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DATA_FLOW_PROBLEM_H
#define MTAC_DATA_FLOW_PROBLEM_H

#include <memory>

#include "mtac/BasicBlock.hpp"

namespace eddic {

namespace mtac {

template<typename DomainValues>
struct Domain {
    boost::optional<DomainValues> values;

    bool top(){
        return !values;
    }
};

template<bool Forward, typename DomainValues>
struct DataFlowProblem {
    //What about statement ?
    //What about reading the basic blocks ?
    
    typedef Domain<DomainValues> ProblemDomain;

    virtual ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) = 0;
    virtual ProblemDomain transfer(std::shared_ptr<BasicBlock> block, ProblemDomain& in) = 0;

    virtual ProblemDomain Boundary() = 0;
    virtual ProblemDomain Init() = 0;
};

} //end of mtac

} //end of eddic

#endif
