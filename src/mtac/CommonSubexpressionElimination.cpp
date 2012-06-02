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

ProblemDomain mtac::CommonSubexpressionElimination::meet(ProblemDomain& in, ProblemDomain& out){
    /*auto result = mtac::union_meet(in, out);

    //TODO

    return result;*/
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
