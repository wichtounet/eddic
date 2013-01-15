//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LIVE_VARIABLE_ANALYSIS_PROBLEM_H
#define MTAC_LIVE_VARIABLE_ANALYSIS_PROBLEM_H

#include <unordered_set>
#include <memory>

#include "assert.hpp"

#include "mtac/DataFlowProblem.hpp"
#include "mtac/EscapeAnalysis.hpp"

namespace eddic {

class Variable;

namespace mtac {
    
typedef std::unordered_set<std::shared_ptr<Variable>> Values;

struct LiveVariableValues {
    Values values;

    void insert(std::shared_ptr<Variable> var){
        values.insert(var);
    }
    
    void erase(std::shared_ptr<Variable> var){
        values.erase(var);
    }

    Values::const_iterator find(std::shared_ptr<Variable> var) const {
        return values.find(var);
    }
    
    Values::const_iterator begin() const {
        return values.begin();
    }
    
    Values::const_iterator end() const {
        return values.end();
    }

    std::size_t size() const {
        return values.size();
    }
};

std::ostream& operator<<(std::ostream& stream, const LiveVariableValues& expression);

struct LiveVariableAnalysisProblem : public DataFlowProblem<DataFlowType::Backward, LiveVariableValues> {
    mtac::EscapedVariables pointer_escaped;
    
    ProblemDomain Boundary(mtac::Function& function) override;
    ProblemDomain Init(mtac::Function& function) override;
   
    void meet(ProblemDomain& in, const ProblemDomain& out) override;

    ProblemDomain transfer(mtac::basic_block_p basic_block, mtac::Quadruple& statement, ProblemDomain& in) override;
    ProblemDomain transfer(mtac::basic_block_p, ltac::Statement&, ProblemDomain&) override { eddic_unreachable("Not LTAC"); };
    
    bool optimize(mtac::Function& function, std::shared_ptr<DataFlowResults<ProblemDomain>> results) override;
    bool optimize(ltac::Statement&, std::shared_ptr<DataFlowResults<ProblemDomain>>) override { eddic_unreachable("Not LTAC"); };
};

} //end of mtac

} //end of eddic

#endif
