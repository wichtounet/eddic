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

bool is_expression(std::shared_ptr<mtac::Quadruple> quadruple){
    return quadruple->op >= mtac::Operator::ADD && quadruple->op <= mtac::Operator::FDIV;
}

ProblemDomain mtac::CommonSubexpressionElimination::transfer(mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto op = (*ptr)->op;
        if(op != mtac::Operator::ARRAY_ASSIGN && op != mtac::Operator::DOT_ASSIGN && op != mtac::Operator::RETURN){
            std::vector<unsigned int> killed;
            
            for(unsigned int i = 0; i < in.values().size(); ++i){
                auto& expression = in.values()[i];

                auto old_size = killed.size();

                if(expression->arg1 && boost::get<std::shared_ptr<Variable>>(&*expression->arg1)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*expression->arg1);

                    if(var == (*ptr)->result){
                        killed.push_back(i);
                    }
                }
                
                if(old_size == killed.size() && expression->arg2 && boost::get<std::shared_ptr<Variable>>(&*expression->arg2)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*expression->arg2);

                    if(var == (*ptr)->result){
                        killed.push_back(i);
                    }
                }
            }

            std::reverse(killed.begin(), killed.end());

            for(auto i : killed){
                out.values().erase(out.values().begin() + i);
            }
        }

        if(is_expression(*ptr)){
            bool exists = false;
            for(auto& expression : out.values()){
                if(are_equivalent(*ptr, expression)){
                    exists = true;
                }
            }

            if(!exists){
                out.values().push_back(*ptr);
            }
        }
    }

    return out;
}

bool mtac::CommonSubexpressionElimination::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& global_results){
    auto& results = global_results->IN_S[statement];

    bool changes = false;

    //TODO

    return changes;
}
