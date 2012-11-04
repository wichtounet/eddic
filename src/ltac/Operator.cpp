//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Operator.hpp"

using namespace eddic;
using eddic::ltac::Operator;

bool ltac::erase_result(ltac::Operator op){
   return op == Operator::MOV 
       || op == Operator::FMOV 
       || op == Operator::XOR 
       || op == Operator::OR 
       || (op >= Operator::LEA && op <= Operator::CMOVLE);
}

bool ltac::erase_result_complete(ltac::Operator op){
    return op == Operator::MOV 
        || op == Operator::FMOV 
        || op == Operator::LEA 
        || op == Operator::MUL;
}
