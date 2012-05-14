//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/ConstantPropagationProblem.hpp"

using namespace eddic;

typedef mtac::ConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::ConstantPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    //TODO
}

ProblemDomain mtac::ConstantPropagationProblem::transfer(mtac::Statement& statement, ProblemDomain& in){
    //TODO
}

ProblemDomain mtac::ConstantPropagationProblem::Boundary(){
    return default_element();
}

ProblemDomain mtac::ConstantPropagationProblem::Init(){
    return top_element();
}
