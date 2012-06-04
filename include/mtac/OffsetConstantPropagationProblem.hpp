//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OFFSET_CONSTANT_PROPAGATION_PROBLEM_H
#define MTAC_OFFSET_CONSTANT_PROPAGATION_PROBLEM_H

#include <unordered_map>
#include <memory>

#include <boost/variant.hpp>

#include "mtac/DataFlowProblem.hpp"
#include "mtac/Offset.hpp"      

namespace eddic {

namespace mtac {

typedef boost::variant<std::string, double, int, std::shared_ptr<Variable>> OffsetConstantValue;
typedef std::unordered_map<Offset, OffsetConstantValue, mtac::OffsetHash> OffsetConstantPropagationValues;

struct OffsetConstantPropagationProblem : public DataFlowProblem<DataFlowType::Forward, OffsetConstantPropagationValues> {
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>>& results);
};

} //end of mtac

} //end of eddic

#endif
