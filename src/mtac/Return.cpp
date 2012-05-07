//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Return.hpp"

using namespace eddic;

mtac::Return::Return(){}
mtac::Return::Return(Argument a1) : arg1(a1) {}
mtac::Return::Return(Argument a1, Argument a2) : arg1(a1), arg2(a2) {}
