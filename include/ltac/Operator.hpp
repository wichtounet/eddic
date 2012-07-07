//=======================================================================
// Copyright Baptiste Wicht 2011.
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

    //Set the memory to 0
    MEMSET,

    //Enter stack frame
    ENTER,

    //Leave stack frame
    LEAVE,

    //Return from function
    RET,

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
    MUL,
    DIV,

    //Float operations
    FADD,
    FSUB,
    FMUL,
    FDIV,

    INC,
    DEC,
    NEG,

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

    NOP
};

} //end of ltac

} //end of eddic

#endif
