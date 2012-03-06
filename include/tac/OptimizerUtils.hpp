//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_COMPILER_H
#define TAC_COMPILER_H

namespace eddic {

namespace tac {

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg){
    visitor.optimized = true;

    quadruple->op = tac::Operator::ASSIGN;
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg, tac::Operator op){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg1, tac::Operator op, tac::Argument arg2){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg1;
    quadruple->arg2 = arg2;
}

} //end of tac

} //end of eddic

#endif
