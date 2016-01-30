//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ltac/Operator.hpp"

using namespace eddic;
using eddic::ltac::Operator;

bool ltac::erase_result(ltac::Operator op){
    return op != Operator::DIV 
        && (
                   op == Operator::MOV 
                || op == Operator::FMOV 
                || op == Operator::MUL3 
                || op == Operator::XOR 
                || op == Operator::OR 
                || (op >= Operator::LEA && op <= Operator::CMOVLE)
         );
}

bool ltac::erase_result_complete(ltac::Operator op){
    return op == Operator::MOV 
        || op == Operator::FMOV 
        || op == Operator::LEA 
        || op == Operator::MUL3;
}
