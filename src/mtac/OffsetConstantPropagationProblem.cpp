//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"

#include "mtac/OffsetConstantPropagationProblem.hpp"

using namespace eddic;

typedef mtac::OffsetConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::OffsetConstantPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    auto result = mtac::union_meet(in, out);

    //Remove all the temporary
    for(auto it = std::begin(result.values()); it != std::end(result.values());){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&it->second)){
            auto variable = *ptr;

            if (variable->position().isTemporary()){
                it = result.values().erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    return result;
}

ProblemDomain mtac::OffsetConstantPropagationProblem::transfer(mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    if(boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = boost::get<std::shared_ptr<mtac::Quadruple>>(statement);

        //Store the value assigned to result+arg1
        if(quadruple->op == mtac::Operator::DOT_ASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                mtac::Offset offset(quadruple->result, *ptr);

                if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                    out[offset] = *ptr;
                } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg2)){
                    out[offset] = *ptr;
                } else if(auto* ptr = boost::get<double>(&*quadruple->arg2)){
                    out[offset] = *ptr;
                } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                    out[offset] = *ptr;
                } else {
                    //The result is not constant at this point
                    out.erase(offset);
                }
            }
        }
    }

    return out;
}

bool mtac::OffsetConstantPropagationProblem::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& global_results){
    auto& results = global_results->IN_S[statement];

    bool changes = false;

    if(boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = boost::get<std::shared_ptr<mtac::Quadruple>>(statement);

        //If constant replace the value assigned to result by the value stored for arg1+arg2
        if(quadruple->op == mtac::Operator::DOT){
            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                mtac::Offset offset(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *ptr);

                if(results.find(offset) != results.end()){
                    quadruple->op = mtac::Operator::ASSIGN;
                    *quadruple->arg1 = results[offset];
                    quadruple->arg2.reset();

                    changes = true;
                }
            }
        }
    }

    return changes;
}
