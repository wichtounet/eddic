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

template<typename T, typename OptVariant>
void if_init(OptVariant& opt_variant, std::function<void(T&)> functor){
    if(opt_variant){
        if(auto* ptr = boost::get<T>(&*opt_variant)){
            functor(*ptr);
        }
    }
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Boundary(mtac::Function& function){
    pointer_escaped = mtac::escape_analysis(function);

    for(auto& block : function){
        for(auto& q : block){
            if(mtac::erase_result(q.op)){
                if(use[block].find(q.result) == use[block].end()){
                    def[block].insert(q.result);
                }
            } else {
                use[block].insert(q.result);
            }

            if_init<std::shared_ptr<Variable>>(q.arg1, [this, &block](std::shared_ptr<Variable>& var){ use[block].insert(var); });
            if_init<std::shared_ptr<Variable>>(q.arg2, [this, &block](std::shared_ptr<Variable>& var){ use[block].insert(var); });
        }
        
        for(auto& escaped_var : *pointer_escaped){
            use[block].insert(escaped_var);
        }
    }

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
        out.values().insert(value);
    }
}

void mtac::LiveVariableAnalysisProblem::transfer(mtac::basic_block_p B, ProblemDomain& x){
    auto& x_values = x.values();

    //Compute x - def(B)

    for(auto& v : def[B]){
        x_values.erase(v);
    }

    //Compute use(B) U (x - def(B))
     
    for(auto& v : use[B]){
        x_values.insert(v);
    }
}

void mtac::LiveVariableAnalysisProblem::transfer(mtac::basic_block_p/* basic_block*/, mtac::Quadruple& quadruple, ProblemDomain& in){
    if(in.top()){
        ProblemDomain::Values values;
        in.int_values = values;
    }

    if(quadruple.op != mtac::Operator::NOP){
        if(mtac::erase_result(quadruple.op)){
            in.values().erase(quadruple.result);
        } else {
            in.values().insert(quadruple.result);
        }

        if_init<std::shared_ptr<Variable>>(quadruple.arg1, [&in](std::shared_ptr<Variable>& var){in.values().insert(var);});
        if_init<std::shared_ptr<Variable>>(quadruple.arg2, [&in](std::shared_ptr<Variable>& var){in.values().insert(var);});
    }
}

bool mtac::operator==(const mtac::Domain<mtac::Values>& lhs, const mtac::Domain<mtac::Values>& rhs){
    if(lhs.top() || rhs.top()){
        return lhs.top() == rhs.top();
    }

    auto& lhs_values = lhs.values();
    auto& rhs_values = rhs.values();

    if(lhs_values.size() != rhs_values.size()){
        return false;
    }

    for(auto& lhs_variable : lhs_values){
        if(rhs_values.find(lhs_variable) == rhs_values.end()){
            return false;
        }
    }

    return true;
}

bool mtac::operator!=(const mtac::Domain<Values>& lhs, const mtac::Domain<Values>& rhs){
    return !(lhs == rhs);
}
