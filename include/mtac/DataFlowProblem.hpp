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
    boost::optional<DomainValues> int_values;

    Domain(){
        //Nothing to init
    }

    Domain(DomainValues&& values) : int_values(values){ //TODO Not sure if it is really useful to have rvalues references here
        //Nothing to init
    }

    DomainValues& values(){
        return *int_values;
    }

    bool top(){
        return !int_values;
    }
};

template<bool Forward, typename DomainValues>
struct DataFlowProblem {
    //What about statement ?
    //What about reading the basic blocks ?
    
    typedef Domain<DomainValues> ProblemDomain;

    virtual ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) = 0;
    virtual ProblemDomain transfer(mtac::Statement& statement, ProblemDomain& in) = 0;

    virtual ProblemDomain Boundary() = 0;
    virtual ProblemDomain Init() = 0;

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(DomainValues());
    }
};

} //end of mtac

} //end of eddic

#endif
