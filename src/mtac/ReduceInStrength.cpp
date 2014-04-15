//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Utils.hpp"

#include "mtac/ReduceInStrength.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

void mtac::ReduceInStrength::operator()(mtac::Quadruple& quadruple){
    switch(quadruple.op){
        case mtac::Operator::MOD:
            if(auto* ptr = boost::get<int>(&*quadruple.arg2)){
                auto constant = *ptr;

                if(isPowerOfTwo(constant)){
                    replaceRight(*this, quadruple, *quadruple.arg1, mtac::Operator::AND, constant - 1);
                }
            }

            break;
        case mtac::Operator::MUL:
            if(*quadruple.arg1 == 2){
                replaceRight(*this, quadruple, *quadruple.arg2, mtac::Operator::ADD, *quadruple.arg2);
            } else if(*quadruple.arg2 == 2){
                replaceRight(*this, quadruple, *quadruple.arg1, mtac::Operator::ADD, *quadruple.arg1);
            }

            break;
        case mtac::Operator::FMUL:
            if(*quadruple.arg1 == 2.0){
                replaceRight(*this, quadruple, *quadruple.arg2, mtac::Operator::FADD, *quadruple.arg2);
            } else if(*quadruple.arg2 == 2.0){
                replaceRight(*this, quadruple, *quadruple.arg1, mtac::Operator::FADD, *quadruple.arg1);
            }

            break;
        default:
            break;
    }
}
