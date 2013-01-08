//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/pure_analysis.hpp"

using namespace eddic;

bool mtac::pure_analysis::operator()(mtac::Program& program){
    //TODO

    //It is only an analysis pass, it is not necessary to restart the other passes
    return false;
}
