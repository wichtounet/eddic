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

#include <boost/utility.hpp>

#define STATIC_CONSTANT(type,name,value) BOOST_STATIC_CONSTANT(type, name = value)

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

struct LiveVariableAnalysisProblem {
    //The type of data managed
    typedef Domain<LiveVariableValues> ProblemDomain;

    //The direction
    STATIC_CONSTANT(DataFlowType, Type, DataFlowType::Backward);

    mtac::EscapedVariables pointer_escaped;
    
    ProblemDomain Boundary(mtac::Function& function);
    ProblemDomain Init(mtac::Function& function);
   
    void meet(ProblemDomain& in, const ProblemDomain& out);
    ProblemDomain transfer(mtac::basic_block_p basic_block, mtac::Quadruple& statement, ProblemDomain& in);
    bool optimize(mtac::Function& function, std::shared_ptr<DataFlowResults<ProblemDomain>> results);

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(ProblemDomain::Values());
    }
};

} //end of mtac

} //end of eddic

#endif
