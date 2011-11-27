//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BASE_STACK_OPERAND_H
#define BASE_STACK_OPERAND_H

#include "il/Operand.hpp"

namespace eddic {

class BaseStackOperand : public Operand {
    private:
        int m_offset;

    public:
        BaseStackOperand(int offset);

        std::string getValue() const ;
};

} //end of eddic

#endif
