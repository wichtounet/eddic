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

#include "mtac/DataFlowProblem.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef boost::variant<std::string, double, int, std::shared_ptr<Variable>> ConstantValue;
typedef std::unordered_map<std::shared_ptr<Variable>, ConstantValue> ConstantPropagationValues;
typedef std::shared_ptr<std::unordered_set<std::shared_ptr<Variable>>> PointerEscaped;

struct ConstantPropagationProblem : public DataFlowProblem<DataFlowType::Forward, ConstantPropagationValues> {
    PointerEscaped pointer_escaped;
    
    void Gather(std::shared_ptr<mtac::Function> function) override;

    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>>& results);
};

} //end of mtac

} //end of eddic

#endif
