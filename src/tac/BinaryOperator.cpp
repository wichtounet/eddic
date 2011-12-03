//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "tac/BinaryOperator.hpp"

using namespace eddic;

tac::BinaryOperator eddic::tac::toBinaryOperator(const std::string& op){
    if(op == "!="){
        return tac::BinaryOperator::EQUALS;
    } else if(op == "=="){
        return tac::BinaryOperator::NOT_EQUALS;
    } else if(op == ">="){
        return tac::BinaryOperator::LESS;
    } else if(op == ">"){
        return tac::BinaryOperator::LESS_EQUALS;
    } else if(op == "<="){
        return tac::BinaryOperator::GREATER;
    } else if(op == "<"){
        return tac::BinaryOperator::GREATER_EQUALS;
    }

    assert(false); //Not handled
}
