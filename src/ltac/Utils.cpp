//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Utils.hpp"

using namespace eddic;

bool eddic::ltac::is_float_operator(mtac::BinaryOperator op){
    return op >= mtac::BinaryOperator::FE && op <= mtac::BinaryOperator::FL;
}

bool eddic::ltac::is_float_var(std::shared_ptr<Variable> variable){
    return variable->type() == BaseType::FLOAT;
}

bool eddic::ltac::is_int_var(std::shared_ptr<Variable> variable){
    return variable->type() == BaseType::INT;
}
