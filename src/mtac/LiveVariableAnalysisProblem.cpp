//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

typedef mtac::LiveVariableAnalysisProblem::ProblemDomain ProblemDomain;

std::ostream& mtac::operator<<(std::ostream& stream, mtac::LiveVariableValues& value){
    stream << "set{";

    for(auto& v : value){
        if(!v){
            stream << "null, ";
        } else {
            stream << v->name() << ", ";
        }
    }

    return stream << "}";
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Boundary(std::shared_ptr<mtac::Function> function){
    pointer_escaped = mtac::escape_analysis(function);

    auto value = default_element();
    return value;
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Init(std::shared_ptr<mtac::Function> /*function*/){
    auto value = default_element();
    return value;
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

namespace {

struct LivenessCollector : public boost::static_visitor<> {
    ProblemDomain& in;

    LivenessCollector(ProblemDomain& in) : in(in) {}

    template<typename Arg>
    inline void update(Arg& arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(in.top()){
                ProblemDomain::Values values;
                in.int_values = values;
            }

            in.values().insert(*ptr);
        }
    }

    template<typename Arg>
    inline void update_optional(Arg& arg){
        if(arg){
            update(*arg);
        }
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        if(quadruple->op != mtac::Operator::NOP){
            if(mtac::erase_result(quadruple->op)){
                in.values().erase(quadruple->result);
            } else {
                in.values().insert(quadruple->result);
            }

            update_optional(quadruple->arg1);
            update_optional(quadruple->arg2);
        }
    }
    
    void operator()(std::shared_ptr<mtac::Param> param){
        update(param->arg);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        update(if_false->arg1);
        update_optional(if_false->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        update(if_->arg1);
        update_optional(if_->arg2);
    }

    template<typename T>
    void operator()(T&){
        //Nothing to do
    }
};

}

ProblemDomain mtac::LiveVariableAnalysisProblem::transfer(std::shared_ptr<mtac::BasicBlock>/* basic_block*/, mtac::Statement& statement, ProblemDomain& out){
    auto in = out;
    
    LivenessCollector collector(in);
    visit(collector, statement);

    for(auto& escaped_var : *pointer_escaped){
        in.values().insert(escaped_var);
    }

    return in;
}

bool mtac::LiveVariableAnalysisProblem::optimize(mtac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> /*global_results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}
