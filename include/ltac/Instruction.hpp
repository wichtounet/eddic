//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_INSTRUCTION_H
#define LTAC_INSTRUCTION_H

#include <memory>

#include <boost/optional.hpp>

#include "ltac/Operator.hpp"
#include "ltac/Argument.hpp"

namespace eddic {

namespace ltac {

enum class Size : char {
    DEFAULT,
    BYTE,
    WORD,
    DOUBLE_WORD,
    QUAD_WORD
};

struct Instruction {
    ltac::Operator op;
    boost::optional<Argument> arg1;
    boost::optional<Argument> arg2;
    boost::optional<Argument> arg3;
    ltac::Size size = ltac::Size::DEFAULT;

    //Instruction should never get copied
    Instruction(const Instruction& rhs) = delete;
    Instruction& operator=(const Instruction& rhs) = delete;

    //Default constructor
    Instruction();

    //Instructions no param
    Instruction(Operator op);

    //Instructions with unary operator
    Instruction(Operator op, Argument arg1);

    //Instructions with binary operator
    Instruction(Operator op, Argument arg1, Argument arg2);

    //Instructions with ternary operator
    Instruction(Operator op, Argument arg1, Argument arg2, Argument arg3);
};

} //end of ltac

} //end of eddic

#endif
