//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef REGISTER_OPERAND_H
#define REGISTER_OPERAND_H

#include "il/Operand.hpp"

namespace eddic {

class RegisterOperand : public Operand {
    private:
        std::string m_register;

    public:
        RegisterOperand(const std::string& reg);

        std::string getValue();
};

} //end of eddic

#endif
