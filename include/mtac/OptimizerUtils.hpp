//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
inline void replaceRight(T& visitor, std::shared_ptr<mtac::Quadruple> quadruple, mtac::Operator op){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1.reset();
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<mtac::Quadruple> quadruple, mtac::Argument arg, mtac::Operator op){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<mtac::Quadruple> quadruple, mtac::Argument arg1, mtac::Operator op, mtac::Argument arg2){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg1;
    quadruple->arg2 = arg2;
}

} //end of mtac

} //end of eddic

#endif
