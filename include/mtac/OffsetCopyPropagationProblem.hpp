//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OFFSET_COPY_PROPAGATION_PROBLEM_H
#define MTAC_OFFSET_COPY_PROPAGATION_PROBLEM_H

#include <unordered_map>
#include <memory>

#include "mtac/DataFlowProblem.hpp"
#include "mtac/Offset.hpp"      

namespace eddic {

namespace mtac {

typedef std::shared_ptr<Variable> OffsetCopyValue;
typedef std::unordered_map<Offset, OffsetCopyValue, mtac::OffsetHash> OffsetCopyPropagationValues;

struct OffsetCopyPropagationProblem : public DataFlowProblem<true, OffsetCopyPropagationValues> {
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>>& results);
};

} //end of mtac

} //end of eddic

#endif
