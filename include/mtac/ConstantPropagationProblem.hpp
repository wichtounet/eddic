//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONSTANT_PROPAGATION_PROBLEM_H
#define MTAC_CONSTANT_PROPAGATION_PROBLEM_H

#include "mtac/DataFlowProblem.hpp"

namespace eddic {

namespace mtac {

typedef int ConstantPropagationDomain;

struct ConstantPropagationProblem : public DataFlowProblem<true, ConstantPropagationDomain> {
    typedef ConstantPropagationDomain Domain;

    Domain meet(Domain& in, Domain& out) override;
    Domain transfer(std::shared_ptr<BasicBlock> block, Domain& in) override;

    Domain Boundary() override;
    Domain Init() override;
};

} //end of mtac

} //end of eddic

#endif
