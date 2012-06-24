//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LIVE_VARIABLE_ANALYSIS_PROBLEM_H
#define MTAC_LIVE_VARIABLE_ANALYSIS_PROBLEM_H

#include <unordered_set>
#include <memory>

#include "mtac/DataFlowProblem.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef std::unordered_set<std::shared_ptr<Variable>> LiveVariableValues;

struct LiveVariableAnalysisProblem : public DataFlowProblem<DataFlowType::Backward, LiveVariableValues> {
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>>& results);
};

} //end of mtac

} //end of eddic

#endif
