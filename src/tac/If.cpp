//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/If.hpp"

using namespace eddic;

tac::If::If(){}
tac::If::If(Argument a1, const std::string& l) : arg1(a1), label(l) {}
tac::If::If(BinaryOperator o, Argument a1, Argument a2, const std::string& l) : arg1(a1), arg2(a2), op(o), label(l) {}