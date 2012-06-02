//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"

#include "mtac/CommonSubexpressionElimination.hpp"

using namespace eddic;

typedef mtac::CommonSubexpressionElimination::ProblemDomain ProblemDomain;

bool is_distributive(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool are_equivalent(std::shared_ptr<mtac::Quadruple> first, std::shared_ptr<mtac::Quadruple> second){
    if(first->op != second->op){
        return false;
    }

    if(is_distributive(first->op)){
        return (*first->arg1 == *second->arg1 && *first->arg2 == *second->arg2) || (*first->arg1 == *second->arg2 && *first->arg2 == *second->arg1);
    } else {
        return (*first->arg1 == *second->arg1 && *first->arg2 == *second->arg2);
    }
}

ProblemDomain mtac::CommonSubexpressionElimination::meet(ProblemDomain& in, ProblemDomain& out){
    ASSERT(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top()){
        return out;
    } else if(out.top()){
        return in;
    } else {
        typename ProblemDomain::Values values;
        ProblemDomain result(values);

        for(auto& in_value : in.values()){
            for(auto& out_value : out.values()){
                if(are_equivalent(in_value, out_value)){
                    values.push_back(in_value);
                }
            }
        }

        return result;
    }
}

ProblemDomain mtac::CommonSubexpressionElimination::transfer(mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    //TODO

    return out;
}

bool mtac::CommonSubexpressionElimination::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& global_results){
    auto& results = global_results->IN_S[statement];

    bool changes = false;

    //TODO

    return changes;
}
