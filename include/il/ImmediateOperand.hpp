//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IMMEDIATE_OPERAND_H
#define IMMEDIATE_OPERAND_H

#include "il/Operand.hpp"

namespace eddic {

class ImmediateIntegerOperand : public Operand {
    private:
        int m_int;

    public:
        ImmediateIntegerOperand(int value);

        std::string getValue();
};

class ImmediateStringOperand : public Operand {
    private:
        std::string m_string;

    public:
        ImmediateStringOperand(std::string value);

        std::string getValue();
};

} //end of eddic

#endif
