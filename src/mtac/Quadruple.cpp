//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Quadruple.hpp"

using namespace eddic;

tac::Quadruple::Quadruple(){}
tac::Quadruple::Quadruple(std::shared_ptr<Variable> r, tac::Argument a1, Operator o) : result(r), arg1(a1), op(o) {}
tac::Quadruple::Quadruple(std::shared_ptr<Variable> r, tac::Argument a1, Operator o, tac::Argument a2) : result(r), arg1(a1), arg2(a2), op(o) {}

tac::Quadruple::Quadruple(Operator o, tac::Argument a1) : arg1(a1), op(o) {}
tac::Quadruple::Quadruple(Operator o, tac::Argument a1, tac::Argument a2) : arg1(a1), arg2(a2), op(o) {}
