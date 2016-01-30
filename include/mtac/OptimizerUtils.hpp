//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_OPTIMIZER_UTILS_H
#define MTAC_OPTIMIZER_UTILS_H

#include <memory>

#include "mtac/Quadruple.hpp"
#include "mtac/Argument.hpp"
#include "mtac/Operator.hpp"

namespace eddic {

namespace mtac {

template<typename T>
inline void replaceRight(T& visitor, mtac::Quadruple& quadruple, mtac::Operator op){
    visitor.optimized = true;

    quadruple.op = op;
    quadruple.arg1.reset();
    quadruple.arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, mtac::Quadruple& quadruple, mtac::Argument arg, mtac::Operator op){
    visitor.optimized = true;

    quadruple.op = op;
    quadruple.arg1 = arg;
    quadruple.arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, mtac::Quadruple& quadruple, mtac::Argument arg1, mtac::Operator op, mtac::Argument arg2){
    visitor.optimized = true;

    quadruple.op = op;
    quadruple.arg1 = arg1;
    quadruple.arg2 = arg2;
}

} //end of mtac

} //end of eddic

#endif
