//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_OPERAND_H
#define GLOBAL_OPERAND_H

#include "il/Operand.hpp"

namespace eddic {

class GlobalOperand : public Operand {
    private:
        std::string m_label;
        int m_offset;

    public:
        GlobalOperand(const std::string& label, int offset = 0);

        std::string getValue();
};

} //end of eddic

#endif
