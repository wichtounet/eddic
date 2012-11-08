//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_OPERATOR_H
#define LTAC_OPERATOR_H

namespace eddic {

namespace ltac {

enum class Operator : unsigned int {
    MOV,
    FMOV,
    MUL3,

    //Set the memory to 0
    MEMSET,

    //Enter stack frame
    ENTER,

    //Leave stack frame
    LEAVE,

    //Return from function
    RET,
    PRE_RET,

    //Comparisons
    CMP_INT,
    CMP_FLOAT,

    //Logical operations
    OR,
    XOR,

    //Stack manipulations
    PUSH,
    POP,
    
    //Load effective address
    LEA,

    //Shifts
    SHIFT_LEFT,
    SHIFT_RIGHT,

    //Math operations
    ADD,
    SUB,
    MUL2,
    DIV,

    //Float operations
    FADD,
    FSUB,
    FMUL,
    FDIV,

    INC,
    DEC,
    NEG,
    NOT,
    AND,

    I2F,
    F2I,

    CMOVE,
    CMOVNE,
    CMOVA,
    CMOVAE,
    CMOVB,
    CMOVBE,
    CMOVG,
    CMOVGE,
    CMOVL,
    CMOVLE,

    //Special placeholders to indicate the start of parameter passing
    PRE_PARAM,

    NOP
};

bool erase_result(ltac::Operator op);
bool erase_result_complete(ltac::Operator op);

} //end of ltac

} //end of eddic

#endif
