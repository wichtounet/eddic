//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H
#define MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H

#include <memory>
#include <unordered_set>
#include <boost/optional.hpp>

#include "assert.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/DataFlowProblem.hpp"
#include "mtac/Quadruple.hpp"

namespace eddic {

namespace mtac {

struct Expression {
    std::shared_ptr<mtac::Quadruple> expression;
    std::shared_ptr<BasicBlock> source;
};

std::ostream& operator<<(std::ostream& stream, Expression& expression);

typedef std::list<Expression> Expressions;

struct CommonSubexpressionElimination : public DataFlowProblem<DataFlowType::Forward, Expressions> {
    std::unordered_set<std::shared_ptr<mtac::Quadruple>> optimized;

    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock>, ltac::Statement&, ProblemDomain&) override { ASSERT_PATH_NOT_TAKEN("Not LTAC"); };
    
    ProblemDomain Init(std::shared_ptr<mtac::Function> function) override;
    ProblemDomain Boundary(std::shared_ptr<mtac::Function> function) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>> results);
    bool optimize(ltac::Statement&, std::shared_ptr<DataFlowResults<ProblemDomain>>) override { ASSERT_PATH_NOT_TAKEN("Not LTAC"); };

    boost::optional<Expressions> init;
};

template<>
struct pass_traits<CommonSubexpressionElimination> {
    STATIC_CONSTANT(pass_type, type, pass_type::DATA_FLOW);
    STATIC_STRING(name, "common_subexpression_elimination");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
