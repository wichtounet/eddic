//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_QUADRUPLE_H
#define TAC_QUADRUPLE_H

#include <unordered_map>
#include <boost/optional.hpp>

namespace eddic {

namespace tac {

template<typename Result, typename Argument, typename Operator>
struct Quadruple {
    Result result;
    boost::optional<Argument> arg1;
    boost::optional<Argument> arg2;
    Operator op;
    
    std::unordered_map<Result, bool> liveness;

    //Quadruple should never get copied
    Quadruple(const Quadruple& rhs) = delete;
    Quadruple& operator=(const Quadruple& rhs) = delete;

    //Default constructor
    Quadruple();

    //Quadruple for unary operators
    Quadruple(Result result, Argument arg1, Operator op);

    //Quadruple for binary operators
    Quadruple(Result result, Argument arg1, Operator op, Argument arg2);

    //Quadruples without assign to result
    Quadruple(Operator op, Argument arg1);

    //Quadruples without assign to result
    Quadruple(Operator op, Argument arg1, Argument arg2);
};

template<typename Result, typename Argument, typename Operator>
tac::Quadruple<Result, Argument, Operator>::Quadruple(){
    //Nothing to init    
}

template<typename Result, typename Argument, typename Operator>
tac::Quadruple<Result, Argument, Operator>::Quadruple(Result result, Argument a1, Operator o) : result(result), arg1(a1), op(o) {
    //Nothing to init    
}

template<typename Result, typename Argument, typename Operator>
tac::Quadruple<Result, Argument, Operator>::Quadruple(Result result, Argument a1, Operator o, Argument a2) : result(result), arg1(a1), arg2(a2), op(o) {
    //Nothing to init    
}

template<typename Result, typename Argument, typename Operator>
tac::Quadruple<Result, Argument, Operator>::Quadruple(Operator o, Argument a1) : arg1(a1), op(o) {
    //Nothing to init    
}

template<typename Result, typename Argument, typename Operator>
tac::Quadruple<Result, Argument, Operator>::Quadruple(Operator o, Argument a1, Argument a2) : arg1(a1), arg2(a2), op(o) {
    //Nothing to init    
}

} //end of tac

} //end of eddic

#endif
