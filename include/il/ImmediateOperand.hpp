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

class ImmediateOperand : public Operand {
    private:
        bool string;
        int m_int;
        std::string m_string;

    public:
        ImmediateOperand(int value);
        ImmediateOperand(std::string value);

        std::string getValue();
};

} //end of eddic

#endif
