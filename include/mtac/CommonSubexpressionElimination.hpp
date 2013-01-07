//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H
#define MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H

#include <memory>
#include <unordered_set>
#include <list>

#include "assert.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/DataFlowProblem.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct Expression {
    std::shared_ptr<mtac::Quadruple> expression;
    basic_block_p source;
};

std::ostream& operator<<(std::ostream& stream, Expression& expression);

typedef std::vector<Expression> Expressions;

struct CommonSubexpressionElimination : public DataFlowProblem<DataFlowType::Forward, Expressions> {
    std::unordered_set<std::shared_ptr<mtac::Quadruple>> optimized;

    void meet(ProblemDomain& in, const ProblemDomain& out) override;

    ProblemDomain transfer(mtac::basic_block_p basic_block, std::shared_ptr<mtac::Quadruple>& statement, ProblemDomain& in) override;
    ProblemDomain transfer(mtac::basic_block_p, ltac::Statement&, ProblemDomain&) override { eddic_unreachable("Not LTAC"); };
    
    ProblemDomain Init(mtac::Function& function) override;
    ProblemDomain Boundary(mtac::Function& function) override;
    
    bool optimize(mtac::Function& function, std::shared_ptr<DataFlowResults<ProblemDomain>> results);
    bool optimize(ltac::Statement&, std::shared_ptr<DataFlowResults<ProblemDomain>>) override { eddic_unreachable("Not LTAC"); };

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
