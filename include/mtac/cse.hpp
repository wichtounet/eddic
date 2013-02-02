//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CSE_H
#define MTAC_CSE_H

#include<memory>

#include "mtac/Quadruple.hpp"
#include "mtac/EscapeAnalysis.hpp"

namespace eddic {

class Variable;
class Type;

namespace mtac {

struct expression {
    std::size_t uid;
    mtac::Argument arg1;
    mtac::Argument arg2;
    mtac::Operator op;
    std::shared_ptr<Variable> tmp;
    std::shared_ptr<const Type> type;

    expression(std::size_t uid, mtac::Argument arg1, mtac::Argument arg2, mtac::Operator op, std::shared_ptr<Variable> tmp, std::shared_ptr<const Type> type) 
            : uid(uid), arg1(arg1), arg2(arg2), op(op), tmp(tmp), type(type) {
        //Nothing
    }
};

bool are_equivalent(mtac::Quadruple& quadruple, expression& exp);
bool is_interesting(mtac::Quadruple& quadruple);
bool is_valid(mtac::Quadruple& quadruple, mtac::EscapedVariables& escaped);

} //end of mtac

} //end of eddic

#endif
