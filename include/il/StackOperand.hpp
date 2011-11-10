//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STACK_OPERAND_H
#define STACK_OPERAND_H

#include "il/Operand.hpp"

namespace eddic {

class StackOperand : public Operand {
    private:
        int m_offset;

    public:
        StackOperand(int offset);

        std::string getValue();
};

} //end of eddic

#endif
