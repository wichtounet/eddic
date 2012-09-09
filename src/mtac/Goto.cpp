//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "mtac/Goto.hpp"

using namespace eddic;

mtac::Goto::Goto(){
    //Nothing to init
}

mtac::Goto::Goto(const std::string& label) : label(label) {
    //Nothing to init
}
