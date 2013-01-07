//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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
#include "mtac/Quadruple.hpp"

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

ProblemDomain mtac::LiveVariableAnalysisProblem::Boundary(mtac::Function& function){
    pointer_escaped = mtac::escape_analysis(function);

    auto value = default_element();
    return value;
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Init(mtac::Function& /*function*/){
    auto value = default_element();
    return value;
}

void mtac::LiveVariableAnalysisProblem::meet(ProblemDomain& out, const ProblemDomain& in){
    if(out.top()){
        out = in;
        return;
    } else if(in.top()){
        //out does not change
        return;
    }

    for(auto& value : in.values()){
        out.values().insert(value);
    }
}

namespace {

struct LivenessCollector {
    ProblemDomain& in;

    LivenessCollector(ProblemDomain& in) : in(in) {}

    template<typename Arg>
    inline void update_optional(Arg& arg){
        if(arg){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*arg)){
                if(in.top()){
                    ProblemDomain::Values values;
                    in.int_values = values;
                }

                in.values().insert(*ptr);
            }
        }
    }

    void collect(std::shared_ptr<mtac::Quadruple>& quadruple){
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
};

} //End of anonymous namespace

ProblemDomain mtac::LiveVariableAnalysisProblem::transfer(mtac::basic_block_p/* basic_block*/, std::shared_ptr<mtac::Quadruple>& statement, ProblemDomain& out){
    auto in = out;
    
    LivenessCollector collector(in);
    collector.collect(statement);

    for(auto& escaped_var : *pointer_escaped){
        in.values().insert(escaped_var);
    }

    return in;
}

bool mtac::LiveVariableAnalysisProblem::optimize(mtac::Function& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> /*global_results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}
