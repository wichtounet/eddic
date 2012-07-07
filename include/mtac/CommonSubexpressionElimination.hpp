//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H
#define MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H

#include <memory>
#include <unordered_map>
#include <boost/optional.hpp>

#include "mtac/DataFlowProblem.hpp"
#include "mtac/Quadruple.hpp"

namespace eddic {

namespace mtac {

struct Expression {
    std::shared_ptr<mtac::Quadruple> expression;
    std::shared_ptr<BasicBlock> source;
};

std::ostream& operator<<(std::ostream& stream, Expression& expression);

typedef std::vector<Expression> Expressions;

struct CommonSubexpressionElimination : public DataFlowProblem<DataFlowType::Forward, Expressions> {
    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> optimized;

    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    
    ProblemDomain Init(std::shared_ptr<mtac::Function> function) override;
    ProblemDomain Boundary(std::shared_ptr<mtac::Function> function) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>>& results);

    boost::optional<Expressions> init;
};

} //end of mtac

} //end of eddic

#endif
