//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Goto.hpp"

using namespace eddic;

tac::Goto::Goto(){}
tac::Goto::Goto(const std::string& l) : label(l) {}