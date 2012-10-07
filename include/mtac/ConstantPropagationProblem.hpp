//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONSTANT_PROPAGATION_PROBLEM_H
#define MTAC_CONSTANT_PROPAGATION_PROBLEM_H

#include <unordered_map>
#include <memory>

#include "variant.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/DataFlowProblem.hpp"
#include "mtac/EscapeAnalysis.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef boost::variant<std::string, double, int, std::shared_ptr<Variable>> ConstantValue;
typedef std::unordered_map<std::shared_ptr<Variable>, ConstantValue> ConstantPropagationValues;

struct ConstantPropagationProblem : public DataFlowProblem<DataFlowType::Forward, ConstantPropagationValues> {
    mtac::EscapedVariables pointer_escaped;
    
    ProblemDomain Boundary(std::shared_ptr<mtac::Function> function) override;
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>> results);
};

template<>
struct pass_traits<ConstantPropagationProblem> {
    STATIC_CONSTANT(pass_type, type, pass_type::DATA_FLOW);
    STATIC_STRING(name, "constant_propagation");
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
    STATIC_CONSTANT(bool, need_configuration, false);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
