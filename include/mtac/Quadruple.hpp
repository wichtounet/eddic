//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_QUADRUPLE_H
#define MTAC_QUADRUPLE_H

#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

#include "mtac/Operator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Variable;

namespace mtac {

struct Quadruple {
    std::shared_ptr<Variable> result;
    boost::optional<mtac::Argument> arg1;
    boost::optional<mtac::Argument> arg2;
    mtac::Operator op;
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    Quadruple(const Quadruple& rhs) = delete;
    Quadruple& operator=(const Quadruple& rhs) = delete;

    //Default constructor
    Quadruple();

    //Quadruple for unary operators
    Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, Operator op);

    //Quadruple for binary operators
    Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, Operator op, mtac::Argument arg2);

    //Quadruples without assign to result
    Quadruple(Operator op, mtac::Argument arg1);

    //Quadruples without assign to result
    Quadruple(Operator op, mtac::Argument arg1, mtac::Argument arg2);
};

} //end of mtac

} //end of eddic

#endif
