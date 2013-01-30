//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H
#define MTAC_COMMON_SUBEXPRESSION_ELIMINATION_H

#include <memory>
#include <unordered_set>
#include <list>

#include <boost/utility.hpp>

#define STATIC_CONSTANT(type,name,value) BOOST_STATIC_CONSTANT(type, name = value)

#include "assert.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/DataFlowProblem.hpp"
#include "mtac/forward.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/EscapeAnalysis.hpp"

namespace eddic {

namespace mtac {

struct Expression {
    std::size_t expression;
    basic_block_p source;
};

std::ostream& operator<<(std::ostream& stream, const Expression& expression);

typedef std::vector<Expression> Expressions;

class CommonSubexpressionElimination {
    public:
        //The type of data managed
        typedef Domain<Expressions> ProblemDomain;

        //The direction
        STATIC_CONSTANT(DataFlowType, Type, DataFlowType::Fast_Forward);
        
        mtac::EscapedVariables pointer_escaped;

        ProblemDomain Init(mtac::Function& function);
        ProblemDomain Boundary(mtac::Function& function);

        void meet(ProblemDomain& in, const ProblemDomain& out);
        void transfer(mtac::basic_block_p basic_block, mtac::Quadruple& statement, ProblemDomain& in);
        bool optimize(mtac::Function& function, std::shared_ptr<DataFlowResults<ProblemDomain>> results);

        boost::optional<Expressions> init;

    private:
        std::unordered_set<std::size_t> optimized;
        mtac::Function* function;
};

template<>
struct pass_traits<CommonSubexpressionElimination> {
    STATIC_CONSTANT(pass_type, type, pass_type::DATA_FLOW);
    STATIC_STRING(name, "common_subexpression_elimination");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

bool operator==(const mtac::Expression& lhs, const mtac::Expression& rhs);
bool operator==(const mtac::Domain<Expressions>& lhs, const mtac::Domain<Expressions>& rhs);
bool operator!=(const mtac::Domain<Expressions>& lhs, const mtac::Domain<Expressions>& rhs);

} //end of mtac

} //end of eddic

#endif
