//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Quadruple.hpp"

using namespace eddic;

mtac::Quadruple::Quadruple(){}
mtac::Quadruple::Quadruple(std::shared_ptr<Variable> r, mtac::Argument a1, Operator o) : result(r), arg1(a1), op(o) {}
mtac::Quadruple::Quadruple(std::shared_ptr<Variable> r, mtac::Argument a1, Operator o, mtac::Argument a2) : result(r), arg1(a1), arg2(a2), op(o) {}

mtac::Quadruple::Quadruple(Operator o, mtac::Argument a1) : arg1(a1), op(o) {}
mtac::Quadruple::Quadruple(Operator o, mtac::Argument a1, mtac::Argument a2) : arg1(a1), arg2(a2), op(o) {}
