//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H
#define MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H

#include <memory>

#include "mtac/DataFlowProblem.hpp"
#include "mtac/Quadruple.hpp"

namespace eddic {

namespace mtac {

struct Expression {
    std::shared_ptr<mtac::Quadruple> expression;
    std::shared_ptr<BasicBlock> source;
};

typedef std::vector<Expression> Expressions;

struct CommonSubexpressionElimination : public DataFlowProblem<DataFlowType::Forward, Expressions> {
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>>& results);
};

} //end of mtac

} //end of eddic

#endif
