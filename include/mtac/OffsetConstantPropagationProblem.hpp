//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_OFFSET_CONSTANT_PROPAGATION_PROBLEM_H
#define MTAC_OFFSET_CONSTANT_PROPAGATION_PROBLEM_H

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "variant.hpp"
#include "Platform.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/DataFlowProblem.hpp"
#include "mtac/Offset.hpp"      
#include "mtac/EscapeAnalysis.hpp"

namespace eddic {

class StringPool;

namespace mtac {

typedef boost::variant<std::string, double, int, std::shared_ptr<Variable>> OffsetConstantValue;
typedef std::unordered_map<Offset, OffsetConstantValue, mtac::OffsetHash> OffsetConstantPropagationValues;

class OffsetConstantPropagationProblem : public DataFlowProblem<DataFlowType::Forward, OffsetConstantPropagationValues> {
    public:
        std::unordered_set<Offset, mtac::OffsetHash> escaped;
        mtac::EscapedVariables pointer_escaped;

        void set_pool(std::shared_ptr<StringPool> string_pool);
        void set_platform(Platform platform);

        ProblemDomain Boundary(std::shared_ptr<mtac::Function> function) override;

        ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
        ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;

        bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>> results);

    private:
        std::shared_ptr<StringPool> string_pool;
        Platform platform;
};

template<>
struct pass_traits<OffsetConstantPropagationProblem> {
    STATIC_CONSTANT(pass_type, type, pass_type::DATA_FLOW);
    STATIC_STRING(name, "constant_propagation");
    STATIC_CONSTANT(bool, need_pool, true);
    STATIC_CONSTANT(bool, need_platform, true);
    STATIC_CONSTANT(unsigned int, todo_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
