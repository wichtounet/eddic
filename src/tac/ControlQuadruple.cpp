//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Quadruple.hpp"

using namespace eddic;

tac::ControlQuadruple::ControlQuadruple(){}
tac::ControlQuadruple::ControlQuadruple(BinaryOperator o, const std::string& l) : op(o), label(l) {}
tac::ControlQuadruple::ControlQuadruple(BinaryOperator o, Argument a1, Argument a2, const std::string& l) : op(o), arg1(a1), arg2(a2), label(l) {}
