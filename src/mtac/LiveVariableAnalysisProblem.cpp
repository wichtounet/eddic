//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
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

ProblemDomain mtac::LiveVariableAnalysisProblem::Boundary(mtac::Function& function){
    pointer_escaped = mtac::escape_analysis(function);

    return ProblemDomain(ProblemDomain::Values());
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Init(mtac::Function& /*function*/){
    return ProblemDomain(ProblemDomain::Values());
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
        out.insert(value);
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

                in.insert(*ptr);
            }
        }
    }

    void collect(mtac::Quadruple& quadruple){
        if(quadruple.op != mtac::Operator::NOP){
            if(mtac::erase_result(quadruple.op)){
                in.erase(quadruple.result);
            } else {
                in.insert(quadruple.result);
            }

            update_optional(quadruple.arg1);
            update_optional(quadruple.arg2);
        }
    }
};

} //End of anonymous namespace

ProblemDomain mtac::LiveVariableAnalysisProblem::transfer(mtac::basic_block_p/* basic_block*/, mtac::Quadruple& statement, ProblemDomain& out){
    auto in = out;
    
    LivenessCollector collector(in);
    collector.collect(statement);

    for(auto& escaped_var : *pointer_escaped){
        in.insert(escaped_var);
    }

    return in;
}
