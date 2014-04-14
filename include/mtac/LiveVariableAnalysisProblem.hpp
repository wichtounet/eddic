//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
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
    
typedef std::set<std::shared_ptr<Variable>> Values;

struct LiveVariableAnalysisProblem {
    //The type of data managed
    typedef Domain<Values> ProblemDomain;

    //The direction and modes
    STATIC_CONSTANT(DataFlowType, Type, DataFlowType::Fast_Backward_Block);
    STATIC_CONSTANT(bool, Low, false);

    mtac::EscapedVariables pointer_escaped;
    
    ProblemDomain Boundary(mtac::Function& function);
    ProblemDomain Init(mtac::Function& function);
   
    void meet(ProblemDomain& in, const ProblemDomain& out);
    
    void transfer(mtac::basic_block_p basic_block, ProblemDomain& in);
    void transfer(mtac::basic_block_p basic_block, mtac::Quadruple& statement, ProblemDomain& in);
    
    std::unordered_map<mtac::basic_block_p, std::set<std::shared_ptr<Variable>>> def;
    std::unordered_map<mtac::basic_block_p, std::set<std::shared_ptr<Variable>>> use;
};

bool operator==(const mtac::Domain<Values>& lhs, const mtac::Domain<Values>& rhs);
bool operator!=(const mtac::Domain<Values>& lhs, const mtac::Domain<Values>& rhs);

} //end of mtac

} //end of eddic

#endif
