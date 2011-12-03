//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Quadruple.hpp"

using namespace eddic;

tac::Quadruple::Quadruple(){}
tac::Quadruple::Quadruple(std::shared_ptr<Variable> r, tac::Argument arg) : result(r), arg1(arg) {}
