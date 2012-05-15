//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"

#include "mtac/ConstantPropagationProblem.hpp"

using namespace eddic;

typedef mtac::ConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::ConstantPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    ASSERT(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top()){
        return out;
    } else if(out.top()){
        return in;
    } else {
        //TODO Find a more proper way to declare that
        ProblemDomain result(in.values());
        result.values().clear();

        auto it = in.values().begin();
        auto end = in.values().end();

        while(it != end){
            auto var = it->first;

            if(out.values().find(var) != out.values().end()){
                auto value_in = it->second;
                auto value_out = out.values()[var];

                if(value_in == value_out){
                    result.values()[var] = value_in;
                }
            }

            ++it;
        }

        return result;
    }
}

//TODO Some cleanup will be necessary
ProblemDomain mtac::ConstantPropagationProblem::transfer(mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    //Only quadruple affects variable
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        if(quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                out.values()[quadruple->result] = *ptr;
            } else if(auto* ptr = boost::get<double>(&*quadruple->arg1)){
                out.values()[quadruple->result] = *ptr;
            } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg1)){
                out.values()[quadruple->result] = *ptr;
            } else {
                //The result is not constant at this point
                out.values().erase(quadruple->result);
            }
        } else {
            auto op = quadruple->op;

            //Check if the operator erase the contents of the result variable
            if(op != mtac::Operator::ARRAY_ASSIGN && op != mtac::Operator::DOT_ASSIGN && op != mtac::Operator::RETURN){
                //The result is not constant at this point
                out.values().erase(quadruple->result);
            }
        }
    }

    return out;
}

ProblemDomain mtac::ConstantPropagationProblem::Boundary(){
    return default_element();
}

ProblemDomain mtac::ConstantPropagationProblem::Init(){
    return top_element();
}
