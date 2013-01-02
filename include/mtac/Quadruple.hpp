//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_QUADRUPLE_H
#define MTAC_QUADRUPLE_H

#include <memory>
#include <boost/optional.hpp>

#include "mtac/Operator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Variable;

namespace mtac {

class basic_block;

enum class Size : char {
    DEFAULT,
    BYTE,
    WORD,
    DOUBLE_WORD,
    QUAD_WORD
};

struct Quadruple {
    std::shared_ptr<Variable> result;
    boost::optional<mtac::Argument> arg1;
    boost::optional<mtac::Argument> arg2;
    mtac::Operator op;
    mtac::Size size = mtac::Size::DEFAULT;
    unsigned int depth;

    std::string param; //For LABEL, GOTO, PARAM
    
    //Filled only in later phase replacing the label
    std::shared_ptr<mtac::basic_block> block;

    //Quadruple should never get copied
    Quadruple(const Quadruple& rhs) = delete;
    Quadruple& operator=(const Quadruple& rhs) = delete;

    //Default constructor
    Quadruple();

    //Quadruples without assign to result and no param
    Quadruple(mtac::Operator op);

    //Quadruple for unary operators
    Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, mtac::Operator op);

    //Quadruple for binary operators
    Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, mtac::Operator op, mtac::Argument arg2);

    //Quadruples without assign to result
    Quadruple(mtac::Operator op, mtac::Argument arg1);

    //Quadruples without assign to result
    Quadruple(mtac::Operator op, mtac::Argument arg1, mtac::Argument arg2);
    
    //Quadruples manipulating labels (reversed param order to not be ambiguous because of std::string)
    Quadruple(const std::string& param, mtac::Operator op);

    const std::string& label() const;
};

} //end of mtac

} //end of eddic

#endif
