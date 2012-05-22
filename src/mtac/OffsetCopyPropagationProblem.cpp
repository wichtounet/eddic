//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"

#include "mtac/OffsetCopyPropagationProblem.hpp"

using namespace eddic;

typedef mtac::OffsetCopyPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::OffsetCopyPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    return mtac::union_meet(in, out);
}

ProblemDomain mtac::OffsetCopyPropagationProblem::transfer(mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    if(boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = boost::get<std::shared_ptr<mtac::Quadruple>>(statement);

        //Store the value assigned to result+arg1
        if(quadruple->op == mtac::Operator::DOT_ASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                mtac::Offset offset(quadruple->result, *ptr);

                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
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

bool mtac::OffsetCopyPropagationProblem::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& global_results){
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
