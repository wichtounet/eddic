//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"

#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

typedef mtac::LiveVariableAnalysisProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::LiveVariableAnalysisProblem::Init(std::shared_ptr<mtac::Function> /*function*/){
    return default_element();
}

ProblemDomain mtac::LiveVariableAnalysisProblem::meet(ProblemDomain& out, ProblemDomain& in){
    if(out.top()){
        return in;
    } else if(in.top()){
        return out;
    }

    typename ProblemDomain::Values values;
    ProblemDomain result(values);

    for(auto& value : in.values()){
        result.values().insert(value);
    }
    
    for(auto& value : out.values()){
        result.values().insert(value);
    }

    return result;
}

template<typename Arg, typename Values>
void update(Arg& arg, Values& values){
    if(mtac::isVariable(arg)){
        auto var = boost::get<std::shared_ptr<Variable>>(arg);
        values.insert(var);
    }
}

template<typename Arg, typename Values>
void update_optional(Arg& arg, Values& values){
    if(arg){
        update(*arg, values);
    }
}

ProblemDomain mtac::LiveVariableAnalysisProblem::transfer(std::shared_ptr<mtac::BasicBlock>/* basic_block*/, mtac::Statement& statement, ProblemDomain& out){
    auto in = out;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;
        
        in.values().erase(quadruple->result);

        update_optional((*ptr)->arg1, in.values());
        update_optional((*ptr)->arg2, in.values());
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        update((*ptr)->arg, in.values());
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
        update((*ptr)->arg1, in.values());
        update_optional((*ptr)->arg2, in.values());
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
        update((*ptr)->arg1, in.values());
        update_optional((*ptr)->arg2, in.values());
    }

    return in;
}

bool mtac::LiveVariableAnalysisProblem::optimize(mtac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& /*global_results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";    
}
